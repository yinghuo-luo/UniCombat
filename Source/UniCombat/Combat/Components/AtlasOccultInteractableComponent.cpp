// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/Components/AtlasOccultInteractableComponent.h"

#include "AbilitySystemComponent.h"
#include "Combat/AtlasGameplayTags.h"
#include "Combat/Library/AtlasCombatBlueprintLibrary.h"

UAtlasOccultInteractableComponent::UAtlasOccultInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = false;
}

void UAtlasOccultInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bCanBeRevealedByWater)
	{
		GrantedInteractionTags.AddTag(TAG_Interaction_Revealable);
	}

	if (bCanReactToBell)
	{
		GrantedInteractionTags.AddTag(TAG_Interaction_BellReactive);
	}

	if (bCanBeExorcisedDirectly)
	{
		GrantedInteractionTags.AddTag(TAG_Interaction_Exorcisable);
	}

	if (bIsRitualTarget)
	{
		GrantedInteractionTags.AddTag(TAG_Interaction_RitualTarget);
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent())
	{
		for (const FGameplayTag& GameplayTag : GrantedInteractionTags)
		{
			AbilitySystemComponent->AddLooseGameplayTag(GameplayTag);
		}
	}
	else
	{
		RuntimeStateTags.AppendTags(GrantedInteractionTags);
	}
}

bool UAtlasOccultInteractableComponent::AreAllRequiredRitualStepsComplete() const
{
	return RequiredRitualStepTags.Num() == 0 || CompletedRitualStepTags.HasAllExact(RequiredRitualStepTags);
}

bool UAtlasOccultInteractableComponent::CanBeRevealed_Implementation(const FAtlasRevealContext& RevealContext) const
{
	return bCanBeRevealedByWater
		&& RevealContext.InstigatorActor != nullptr
		&& !HasStateTag(TAG_State_Revealed);
}

void UAtlasOccultInteractableComponent::HandleRevealTriggered_Implementation(const FAtlasRevealContext& RevealContext)
{
	FGameplayTagContainer TagsToAdd = RevealGrantedTags;
	if (TagsToAdd.Num() == 0)
	{
		TagsToAdd.AddTag(TAG_State_Revealed);
	}

	AddStateTags(TagsToAdd);
	if (GetOwner() != nullptr && UAtlasCombatBlueprintLibrary::ActorHasMatchingGameplayTag(GetOwner(), TAG_Rule_WeakToReveal))
	{
		FGameplayTagContainer BrokenVeilTags;
		BrokenVeilTags.AddTag(TAG_State_BrokenVeil);
		AddStateTags(BrokenVeilTags);
	}
}

FGameplayTagContainer UAtlasOccultInteractableComponent::GetRevealResponseTags_Implementation() const
{
	return RuntimeStateTags;
}

bool UAtlasOccultInteractableComponent::CanReactToSoulBell_Implementation(const FAtlasSoulBellContext& BellContext) const
{
	return bCanReactToBell && BellContext.InstigatorActor != nullptr;
}

void UAtlasOccultInteractableComponent::HandleSoulBellTriggered_Implementation(const FAtlasSoulBellContext& BellContext)
{
	FGameplayTagContainer TagsToAdd = SoulBellGrantedTags;
	if (TagsToAdd.Num() == 0)
	{
		TagsToAdd.AddTag(TAG_State_Stabilized);
	}

	if (bBellOpensExorciseWindow || BellContext.bRequestsExecutionWindow)
	{
		TagsToAdd.AddTag(TAG_State_Executable);
		TagsToAdd.AddTag(TAG_State_ExorciseWindow);
	}

	AddStateTags(TagsToAdd);
}

bool UAtlasOccultInteractableComponent::CanOpenExecutionWindowFromBell_Implementation() const
{
	return bBellOpensExorciseWindow;
}

FGameplayTagContainer UAtlasOccultInteractableComponent::GetSoulBellResponseTags_Implementation() const
{
	return RuntimeStateTags;
}

bool UAtlasOccultInteractableComponent::CanBeExorcised_Implementation(const FAtlasExorciseContext& ExorciseContext) const
{
	return ExorciseContext.InstigatorActor != nullptr
		&& (bCanBeExorcisedDirectly || IsExorciseWindowOpen_Implementation() || ExorciseContext.bIgnoreWindowRequirement);
}

bool UAtlasOccultInteractableComponent::IsExorciseWindowOpen_Implementation() const
{
	return HasStateTag(TAG_State_ExorciseWindow) || HasStateTag(TAG_State_Executable);
}

void UAtlasOccultInteractableComponent::HandleExorcised_Implementation(const FAtlasExorciseContext& ExorciseContext)
{
	FGameplayTagContainer TagsToRemove;
	TagsToRemove.AddTag(TAG_State_Executable);
	TagsToRemove.AddTag(TAG_State_ExorciseWindow);
	TagsToRemove.AddTag(TAG_State_BrokenVeil);
	RemoveStateTags(TagsToRemove);

	if (ExorciseContext.bForceKill)
	{
		FGameplayTagContainer DeadTags;
		DeadTags.AddTag(TAG_State_Dead);
		AddStateTags(DeadTags);
	}
}

bool UAtlasOccultInteractableComponent::CanAcceptRitualStep_Implementation(FGameplayTag RitualStepTag,
	const FAtlasRitualContext& RitualContext) const
{
	return bIsRitualTarget
		&& RitualStepTag.IsValid()
		&& RitualContext.InstigatorActor != nullptr
		&& RequiredRitualStepTags.HasTagExact(RitualStepTag);
}

void UAtlasOccultInteractableComponent::HandleRitualStepApplied_Implementation(FGameplayTag RitualStepTag,
	const FAtlasRitualContext& RitualContext)
{
	if (!RitualStepTag.IsValid())
	{
		return;
	}

	CompletedRitualStepTags.AddTag(RitualStepTag);
	if (AreAllRequiredRitualStepsComplete())
	{
		AddStateTags(RitualCompletedGrantedTags);
	}
}

FGameplayTagContainer UAtlasOccultInteractableComponent::GetCompletedRitualStepTags_Implementation() const
{
	return CompletedRitualStepTags;
}

UAbilitySystemComponent* UAtlasOccultInteractableComponent::GetOwnerAbilitySystemComponent() const
{
	return UAtlasCombatBlueprintLibrary::GetAbilitySystemFromActor(GetOwner());
}

void UAtlasOccultInteractableComponent::AddStateTags(const FGameplayTagContainer& TagsToAdd)
{
	if (UAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent())
	{
		for (const FGameplayTag& GameplayTag : TagsToAdd)
		{
			AbilitySystemComponent->AddLooseGameplayTag(GameplayTag);
		}
	}

	RuntimeStateTags.AppendTags(TagsToAdd);
}

void UAtlasOccultInteractableComponent::RemoveStateTags(const FGameplayTagContainer& TagsToRemove)
{
	if (UAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent())
	{
		for (const FGameplayTag& GameplayTag : TagsToRemove)
		{
			if (AbilitySystemComponent->HasMatchingGameplayTag(GameplayTag))
			{
				AbilitySystemComponent->RemoveLooseGameplayTag(GameplayTag);
			}
		}
	}

	for (const FGameplayTag& GameplayTag : TagsToRemove)
	{
		RuntimeStateTags.RemoveTag(GameplayTag);
	}
}

bool UAtlasOccultInteractableComponent::HasStateTag(const FGameplayTag TagToFind) const
{
	if (!TagToFind.IsValid())
	{
		return false;
	}

	if (UAbilitySystemComponent* AbilitySystemComponent = GetOwnerAbilitySystemComponent())
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(TagToFind))
		{
			return true;
		}
	}

	return RuntimeStateTags.HasTag(TagToFind);
}
