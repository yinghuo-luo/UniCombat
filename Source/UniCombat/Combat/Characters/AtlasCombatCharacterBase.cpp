// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/Characters/AtlasCombatCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Combat/AbilitySystem/AtlasAbilitySystemComponent.h"
#include "Combat/AbilitySystem/AtlasCombatAttributeSet.h"
#include "Combat/AtlasGameplayTags.h"
#include "Combat/Data/AtlasAbilitySet.h"
#include "Combat/Data/AtlasCharacterConfigData.h"
#include "Combat/TalentTree/AtlasTalentTreeComponent.h"
#include "Combat/Weapons/AtlasWeaponComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

AAtlasCombatCharacterBase::AAtlasCombatCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAtlasAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	CombatAttributes = CreateDefaultSubobject<UAtlasCombatAttributeSet>(TEXT("CombatAttributes"));
	WeaponComponent = CreateDefaultSubobject<UAtlasWeaponComponent>(TEXT("WeaponComponent"));
	TalentTreeComponent = CreateDefaultSubobject<UAtlasTalentTreeComponent>(TEXT("TalentTreeComponent"));
}

void AAtlasCombatCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeCombatActor();
}

void AAtlasCombatCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeCombatActor();
}

UAbilitySystemComponent* AAtlasCombatCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAtlasCombatCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAtlasCombatCharacterBase, bIsDead);
}

bool AAtlasCombatCharacterBase::TryActivateAbilityByInputTag(const FGameplayTag InputTag)
{
	return AbilitySystemComponent != nullptr && AbilitySystemComponent->TryActivateAbilityByInputTag(InputTag);
}

void AAtlasCombatCharacterBase::ReleaseAbilityInputTag(const FGameplayTag InputTag)
{
	if (AbilitySystemComponent != nullptr)
	{
		AbilitySystemComponent->ReleaseAbilityByInputTag(InputTag);
	}
}

bool AAtlasCombatCharacterBase::TryActivateAbilityByTag(const FGameplayTag AbilityTag)
{
	return AbilitySystemComponent != nullptr && AbilitySystemComponent->TryActivateAbilityByTagExact(AbilityTag);
}

float AAtlasCombatCharacterBase::GetCurrentHealth() const
{
	return CombatAttributes != nullptr ? CombatAttributes->GetHealth() : 0.0f;
}

float AAtlasCombatCharacterBase::GetMaxHealthValue() const
{
	return CombatAttributes != nullptr ? CombatAttributes->GetMaxHealth() : 0.0f;
}

void AAtlasCombatCharacterBase::InitializeCombatActor()
{
	if (bCombatInitialized || AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->InitializeAbilityActorInfo(this, this);
	
	ApplyCharacterConfig();
	
	bCombatInitialized = true;
}

void AAtlasCombatCharacterBase::ApplyCharacterConfig()
{
	if (CharacterConfig == nullptr || AbilitySystemComponent == nullptr)
	{
		return;
	}

	CombatFaction = CharacterConfig->DefaultFaction;
	TargetCategory = CharacterConfig->TargetCategory;
	DefaultCombatTags = CharacterConfig->DefaultUnitTags;

	AbilitySystemComponent->AddLooseGameplayTag(GetFactionTag());
	for (const FGameplayTag& Tag : DefaultCombatTags)
	{
		AbilitySystemComponent->AddLooseGameplayTag(Tag);
	}

	if (HasAuthority())
	{
		//注册GA
		AbilitySystemComponent->GrantAbilitySet(CharacterConfig->DefaultAbilitySet, this);
	}
}

EAtlasCombatFaction AAtlasCombatCharacterBase::GetCombatFaction_Implementation() const
{
	return CombatFaction;
}

bool AAtlasCombatCharacterBase::IsAlive_Implementation() const
{
	return !bIsDead && CombatAttributes != nullptr && CombatAttributes->GetHealth() > 0.0f;
}

bool AAtlasCombatCharacterBase::IsCombatTargetable_Implementation() const
{
	return !bIsDead;
}

UAbilitySystemComponent* AAtlasCombatCharacterBase::GetCombatAbilitySystemComponent_Implementation() const
{
	return AbilitySystemComponent;
}

FGameplayTagContainer AAtlasCombatCharacterBase::GetOwnedCombatTags_Implementation() const
{
	FGameplayTagContainer TagContainer;
	if (AbilitySystemComponent != nullptr)
	{
		AbilitySystemComponent->GetOwnedGameplayTags(TagContainer);
	}
	return TagContainer;
}

FVector AAtlasCombatCharacterBase::GetCombatAimPoint_Implementation(FName PointName) const
{
	if (GetMesh() != nullptr && PointName != NAME_None && GetMesh()->DoesSocketExist(PointName))
	{
		return GetMesh()->GetSocketLocation(PointName);
	}

	return GetActorLocation() + FVector(0.0f, 0.0f, BaseEyeHeight);
}

bool AAtlasCombatCharacterBase::CanBeLockedOn_Implementation() const
{
	return !bIsDead;
}

FVector AAtlasCombatCharacterBase::GetLockTargetLocation_Implementation() const
{
	if (CharacterConfig != nullptr)
	{
		if (GetMesh() != nullptr && CharacterConfig->LockSocketName != NAME_None
			&& GetMesh()->DoesSocketExist(CharacterConfig->LockSocketName))
		{
			return GetMesh()->GetSocketLocation(CharacterConfig->LockSocketName) + CharacterConfig->LockOffset;
		}

		return GetActorLocation() + CharacterConfig->LockOffset;
	}

	return GetCombatAimPoint_Implementation(NAME_None);
}

FText AAtlasCombatCharacterBase::GetTargetDisplayName_Implementation() const
{
	if (CharacterConfig != nullptr && !CharacterConfig->DisplayName.IsEmpty())
	{
		return CharacterConfig->DisplayName;
	}

	return FText::FromString(GetName());
}

EAtlasTargetCategory AAtlasCombatCharacterBase::GetTargetCategory_Implementation() const
{
	return TargetCategory;
}

void AAtlasCombatCharacterBase::HandleGameplayHit_Implementation(const FAtlasCombatHitData& HitData)
{
	if (bIsDead)
	{
		return;
	}

	BP_OnCombatHit(HitData);

	if (!HitData.bIsFatal && CanBeInterruptedByHit_Implementation(HitData) && AbilitySystemComponent != nullptr)
	{
		AbilitySystemComponent->TryActivateAbilityByTagExact(TAG_Ability_Common_HitReact);
	}
}

void AAtlasCombatCharacterBase::HandleGameplayDeath_Implementation(const FAtlasCombatHitData& HitData)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	if (AbilitySystemComponent != nullptr)
	{
		AbilitySystemComponent->AddLooseGameplayTag(TAG_State_Dead);
		AbilitySystemComponent->TryActivateAbilityByTagExact(TAG_Ability_Common_Death);
	}

	OnDeathStarted(HitData);
	BP_OnCombatDeath(HitData);
}

bool AAtlasCombatCharacterBase::CanBeInterruptedByHit_Implementation(const FAtlasCombatHitData& HitData) const
{
	return AbilitySystemComponent != nullptr
		&& !AbilitySystemComponent->HasMatchingGameplayTag(TAG_State_SuperArmor)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(TAG_State_Invincible)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(TAG_State_Dead);
}

void AAtlasCombatCharacterBase::OnDeathStarted_Implementation(const FAtlasCombatHitData& HitData)
{
	GetCharacterMovement()->DisableMovement();
}

void AAtlasCombatCharacterBase::OnRep_IsDead()
{
	if (bIsDead)
	{
		GetCharacterMovement()->DisableMovement();
	}
}


FGameplayTag AAtlasCombatCharacterBase::GetFactionTag() const
{
	switch (CombatFaction)
	{
	case EAtlasCombatFaction::Player:
		return TAG_Faction_Player;
	case EAtlasCombatFaction::Enemy:
		return TAG_Faction_Enemy;
	case EAtlasCombatFaction::Summon:
		return TAG_Faction_Summon;
	default:
		return TAG_Faction_Neutral;
	}
}
