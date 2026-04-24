// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/AbilitySystem/AtlasAbilitySystemComponent.h"

#include "Combat/Data/AtlasAbilitySet.h"
#include "GameplayEffect.h"

UAtlasAbilitySystemComponent::UAtlasAbilitySystemComponent()
{
	SetIsReplicatedByDefault(true);
	SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void UAtlasAbilitySystemComponent::InitializeAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	RefreshAbilityActorInfo();
}

bool UAtlasAbilitySystemComponent::TryActivateAbilityByInputTag(const FGameplayTag InputTag,
	const bool bAllowRemoteActivation)
{
	return AbilityInputTagPressed(InputTag, bAllowRemoteActivation);
}

void UAtlasAbilitySystemComponent::ReleaseAbilityByInputTag(const FGameplayTag InputTag)
{
	AbilityInputTagReleased(InputTag);
}

bool UAtlasAbilitySystemComponent::TryActivateAbilityByTagExact(const FGameplayTag AbilityTag,
	const bool bAllowRemoteActivation)
{
	if (!AbilityTag.IsValid())
	{
		return false;
	}

	bool bActivatedAnyAbility = false;

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		
		if (AbilitySpec.Ability != nullptr && 
			AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(AbilityTag))
		{
			bActivatedAnyAbility |= TryActivateAbility(AbilitySpec.Handle, bAllowRemoteActivation);
		}
	}

	return bActivatedAnyAbility;
}

void UAtlasAbilitySystemComponent::GrantAbilitySet(const UAtlasAbilitySet* AbilitySet, UObject* SourceObject,
	FAtlasAbilitySet_GrantedHandles* OutGrantedHandles)
{
	if (AbilitySet != nullptr)
	{
		AbilitySet->GiveToAbilitySystem(this, SourceObject, OutGrantedHandles);
	}
}

bool UAtlasAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag InputTag,
	const bool bAllowRemoteActivation)
{
	if (!InputTag.IsValid())
	{
		return false;
	}

	bool bActivatedAnyAbility = false;
	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		AbilitySpec.InputPressed = true;

		if (AbilitySpec.IsActive())
		{
			AbilitySpecInputPressed(AbilitySpec);
			continue;
		}

		bActivatedAnyAbility |= TryActivateAbility(AbilitySpec.Handle, bAllowRemoteActivation);
	}

	return bActivatedAnyAbility;
}

void UAtlasAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	ABILITYLIST_SCOPE_LOCK();

	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		AbilitySpec.InputPressed = false;

		if (AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
		if (AbilitySpec.Ability)
		{
			CancelAbilityHandle(AbilitySpec.Handle);
		}
	}
}
