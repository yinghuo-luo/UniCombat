#include "Combat/TalentTree/AtlasTalentTreeComponent.h"

#include "Combat/AbilitySystem/AtlasAbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Net/UnrealNetwork.h"

UAtlasTalentTreeComponent::UAtlasTalentTreeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UAtlasTalentTreeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() != nullptr && GetOwner()->HasAuthority())
	{
		RebuildGrantedTalents();
	}
}

void UAtlasTalentTreeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetOwner() != nullptr && GetOwner()->HasAuthority())
	{
		ClearGrantedTalents();
	}

	Super::EndPlay(EndPlayReason);
}

void UAtlasTalentTreeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAtlasTalentTreeComponent, UnspentTalentPoints);
	DOREPLIFETIME(UAtlasTalentTreeComponent, TalentStates);
}

int32 UAtlasTalentTreeComponent::GetTalentRank(const FGameplayTag TalentTag) const
{
	const int32 StateIndex = FindTalentStateIndex(TalentTag);
	return TalentStates.IsValidIndex(StateIndex)
		? TalentStates[StateIndex].CurrentRank
		: 0;
}

bool UAtlasTalentTreeComponent::IsTalentUnlocked(const FGameplayTag TalentTag) const
{
	return GetTalentRank(TalentTag) > 0;
}

bool UAtlasTalentTreeComponent::HasRequiredOwnerTags(const FGameplayTagContainer& RequiredTags) const
{
	if (RequiredTags.IsEmpty())
	{
		return true;
	}

	FGameplayTagContainer OwnedTags;
	if (const UAtlasAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent())
	{
		AbilitySystemComponent->GetOwnedGameplayTags(OwnedTags);
	}

	return OwnedTags.HasAll(RequiredTags);
}

void UAtlasTalentTreeComponent::AddTalentPoints(const int32 PointsToAdd)
{
	if (PointsToAdd <= 0 || GetOwner() == nullptr)
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		ServerAddTalentPoints(PointsToAdd);
		return;
	}

	UnspentTalentPoints += PointsToAdd;
	OnTalentTreeChanged.Broadcast();
	GetOwner()->ForceNetUpdate();
}

void UAtlasTalentTreeComponent::ServerAddTalentPoints_Implementation(const int32 PointsToAdd)
{
	AddTalentPoints(PointsToAdd);
}

bool UAtlasTalentTreeComponent::CanUpgradeTalent(const FGameplayTag TalentTag, FText& OutFailureReason) const
{
	const FAtlasTalentNodeDefinition* TalentDefinition = FindTalentDefinition(TalentTag);
	if (TalentDefinition == nullptr)
	{
		OutFailureReason = FText::FromString(TEXT("Talent definition not found."));
		return false;
	}

	if (TalentDefinition->RankDefinitions.IsEmpty())
	{
		OutFailureReason = FText::FromString(TEXT("Talent has no ranks configured."));
		return false;
	}

	const int32 CurrentRank = GetTalentRank(TalentTag);
	if (CurrentRank >= TalentDefinition->RankDefinitions.Num())
	{
		OutFailureReason = FText::FromString(TEXT("Talent is already at max rank."));
		return false;
	}

	if (!HasRequiredOwnerTags(TalentDefinition->RequiredOwnerTags))
	{
		OutFailureReason = FText::FromString(TEXT("Owner does not meet tag requirements."));
		return false;
	}

	if (!SatisfiesPrerequisites(*TalentDefinition))
	{
		OutFailureReason = FText::FromString(TEXT("Talent prerequisites are not satisfied."));
		return false;
	}

	const int32 Cost = FMath::Max(TalentDefinition->RankDefinitions[CurrentRank].Cost, 1);
	if (UnspentTalentPoints < Cost)
	{
		OutFailureReason = FText::FromString(TEXT("Not enough talent points."));
		return false;
	}

	OutFailureReason = FText::GetEmpty();
	return true;
}

bool UAtlasTalentTreeComponent::UpgradeTalent(const FGameplayTag TalentTag)
{
	if (GetOwner() == nullptr)
	{
		return false;
	}

	if (!GetOwner()->HasAuthority())
	{
		ServerUpgradeTalent(TalentTag);
		return true;
	}

	FText FailureReason;
	if (!CanUpgradeTalent(TalentTag, FailureReason))
	{
		return false;
	}

	const FAtlasTalentNodeDefinition* TalentDefinition = FindTalentDefinition(TalentTag);
	if (TalentDefinition == nullptr)
	{
		return false;
	}

	int32 StateIndex = FindTalentStateIndex(TalentTag);
	const int32 PreviousRank = StateIndex != INDEX_NONE
		? TalentStates[StateIndex].CurrentRank
		: 0;
	const int32 RankCost = FMath::Max(TalentDefinition->RankDefinitions[PreviousRank].Cost, 1);
	const int32 NewRank = PreviousRank + 1;

	if (StateIndex == INDEX_NONE)
	{
		FAtlasTalentNodeState NewState;
		NewState.TalentTag = TalentTag;
		NewState.CurrentRank = NewRank;
		StateIndex = TalentStates.Add(NewState);
	}
	else
	{
		TalentStates[StateIndex].CurrentRank = NewRank;
	}

	UnspentTalentPoints -= RankCost;
	ApplyTalentRank(*TalentDefinition, PreviousRank);
	BroadcastTalentStateChanged(TalentTag, NewRank);
	GetOwner()->ForceNetUpdate();
	return true;
}

void UAtlasTalentTreeComponent::ServerUpgradeTalent_Implementation(const FGameplayTag TalentTag)
{
	UpgradeTalent(TalentTag);
}

void UAtlasTalentTreeComponent::ResetTalents()
{
	if (GetOwner() == nullptr)
	{
		return;
	}

	if (!GetOwner()->HasAuthority())
	{
		ServerResetTalents();
		return;
	}

	int32 RefundedPoints = 0;
	for (const FAtlasTalentNodeState& TalentState : TalentStates)
	{
		if (const FAtlasTalentNodeDefinition* TalentDefinition = FindTalentDefinition(TalentState.TalentTag))
		{
			const int32 AppliedRanks = FMath::Min(TalentState.CurrentRank, TalentDefinition->RankDefinitions.Num());
			for (int32 RankIndex = 0; RankIndex < AppliedRanks; ++RankIndex)
			{
				RefundedPoints += FMath::Max(TalentDefinition->RankDefinitions[RankIndex].Cost, 1);
			}
		}
	}

	ClearGrantedTalents();
	TalentStates.Reset();
	UnspentTalentPoints += RefundedPoints;
	OnTalentTreeChanged.Broadcast();
	GetOwner()->ForceNetUpdate();
}

void UAtlasTalentTreeComponent::ServerResetTalents_Implementation()
{
	ResetTalents();
}

void UAtlasTalentTreeComponent::OnRep_UnspentTalentPoints()
{
	OnTalentTreeChanged.Broadcast();
}

void UAtlasTalentTreeComponent::OnRep_TalentStates()
{
	OnTalentTreeChanged.Broadcast();
}

const FAtlasTalentNodeDefinition* UAtlasTalentTreeComponent::FindTalentDefinition(const FGameplayTag TalentTag) const
{
	return TalentTreeData != nullptr
		? TalentTreeData->FindTalentDefinition(TalentTag)
		: nullptr;
}

int32 UAtlasTalentTreeComponent::FindTalentStateIndex(const FGameplayTag TalentTag) const
{
	return TalentStates.IndexOfByPredicate(
		[TalentTag](const FAtlasTalentNodeState& State)
		{
			return State.TalentTag == TalentTag;
		});
}

UAtlasAbilitySystemComponent* UAtlasTalentTreeComponent::GetOwnerAbilitySystemComponent() const
{
	return GetOwner() != nullptr
		? GetOwner()->FindComponentByClass<UAtlasAbilitySystemComponent>()
		: nullptr;
}

bool UAtlasTalentTreeComponent::SatisfiesPrerequisites(const FAtlasTalentNodeDefinition& TalentDefinition) const
{
	if (TalentDefinition.PrerequisiteTalentTags.IsEmpty())
	{
		return true;
	}

	if (TalentDefinition.bRequireAllPrerequisites)
	{
		for (const FGameplayTag& PrerequisiteTag : TalentDefinition.PrerequisiteTalentTags)
		{
			if (GetTalentRank(PrerequisiteTag) <= 0)
			{
				return false;
			}
		}

		return true;
	}

	for (const FGameplayTag& PrerequisiteTag : TalentDefinition.PrerequisiteTalentTags)
	{
		if (GetTalentRank(PrerequisiteTag) > 0)
		{
			return true;
		}
	}

	return false;
}

void UAtlasTalentTreeComponent::RebuildGrantedTalents()
{
	ClearGrantedTalents();

	if (GetOwner() == nullptr || !GetOwner()->HasAuthority())
	{
		return;
	}

	for (const FAtlasTalentNodeState& TalentState : TalentStates)
	{
		const FAtlasTalentNodeDefinition* TalentDefinition = FindTalentDefinition(TalentState.TalentTag);
		if (TalentDefinition == nullptr)
		{
			continue;
		}

		const int32 RankCount = FMath::Min(TalentState.CurrentRank, TalentDefinition->RankDefinitions.Num());
		for (int32 RankIndex = 0; RankIndex < RankCount; ++RankIndex)
		{
			ApplyTalentRank(*TalentDefinition, RankIndex);
		}
	}
}

void UAtlasTalentTreeComponent::ClearGrantedTalents()
{
	UAtlasAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent();

	for (TPair<FGameplayTag, FAtlasAppliedTalentNodeState>& AppliedEntry : AppliedTalentStates)
	{
		if (AbilitySystemComponent != nullptr)
		{
			for (FAtlasAppliedTalentRankState& RankState : AppliedEntry.Value.AppliedRanks)
			{
				RankState.GrantedHandles.TakeFromAbilitySystem(AbilitySystemComponent);

				for (const FActiveGameplayEffectHandle EffectHandle : RankState.GrantedEffectHandles)
				{
					AbilitySystemComponent->RemoveActiveGameplayEffect(EffectHandle);
				}

				for (const FGameplayTag& GrantedTag : RankState.GrantedTags)
				{
					if (AbilitySystemComponent->HasMatchingGameplayTag(GrantedTag))
					{
						AbilitySystemComponent->RemoveLooseGameplayTag(GrantedTag);
					}
				}
			}

			if (AppliedEntry.Value.bGrantedTalentTag && AppliedEntry.Key.IsValid()
				&& AbilitySystemComponent->HasMatchingGameplayTag(AppliedEntry.Key))
			{
				AbilitySystemComponent->RemoveLooseGameplayTag(AppliedEntry.Key);
			}
		}
	}

	AppliedTalentStates.Reset();
}

void UAtlasTalentTreeComponent::ApplyTalentRank(const FAtlasTalentNodeDefinition& TalentDefinition, const int32 RankIndex)
{
	UAtlasAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent();
	if (AbilitySystemComponent == nullptr || !TalentDefinition.RankDefinitions.IsValidIndex(RankIndex))
	{
		return;
	}

	FAtlasAppliedTalentNodeState& AppliedNodeState = AppliedTalentStates.FindOrAdd(TalentDefinition.TalentTag);
	if (!AppliedNodeState.bGrantedTalentTag && TalentDefinition.TalentTag.IsValid())
	{
		AbilitySystemComponent->AddLooseGameplayTag(TalentDefinition.TalentTag);
		AppliedNodeState.bGrantedTalentTag = true;
	}

	const FAtlasTalentRankDefinition& RankDefinition = TalentDefinition.RankDefinitions[RankIndex];
	FAtlasAppliedTalentRankState AppliedRankState;

	if (RankDefinition.GrantedAbilitySet != nullptr)
	{
		AbilitySystemComponent->GrantAbilitySet(
			RankDefinition.GrantedAbilitySet,
			this,
			&AppliedRankState.GrantedHandles);
	}

	for (const TSubclassOf<UGameplayEffect>& EffectClass : RankDefinition.GrantedEffects)
	{
		if (EffectClass == nullptr)
		{
			continue;
		}

		const FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		const FGameplayEffectSpecHandle EffectSpec = AbilitySystemComponent->MakeOutgoingSpec(
			EffectClass,
			RankDefinition.EffectLevel,
			EffectContext);
		if (!EffectSpec.IsValid())
		{
			continue;
		}

		AppliedRankState.GrantedEffectHandles.Add(
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get()));
	}

	for (const FGameplayTag& GrantedTag : RankDefinition.GrantedTags)
	{
		AbilitySystemComponent->AddLooseGameplayTag(GrantedTag);
		AppliedRankState.GrantedTags.AddTag(GrantedTag);
	}

	AppliedNodeState.AppliedRanks.Add(MoveTemp(AppliedRankState));
}

void UAtlasTalentTreeComponent::BroadcastTalentStateChanged(const FGameplayTag TalentTag, const int32 NewRank)
{
	OnTalentNodeRankChanged.Broadcast(TalentTag, NewRank);
	OnTalentTreeChanged.Broadcast();
}
