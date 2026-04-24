// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/Data/AtlasAbilitySet.h"

#include "AbilitySystemComponent.h"
#include "Combat/AbilitySystem/AtlasAbilitySystemComponent.h"

void FAtlasAbilitySet_GrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		AbilitySpecHandles.Add(Handle);
	}
}

void FAtlasAbilitySet_GrantedHandles::AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle)
{
	if (Handle.IsValid())
	{
		GameplayEffectHandles.Add(Handle);
	}
}

void FAtlasAbilitySet_GrantedHandles::TakeFromAbilitySystem(UAtlasAbilitySystemComponent* AbilitySystemComponent)
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilitySpecHandles)
	{
		AbilitySystemComponent->ClearAbility(Handle);
	}

	for (const FActiveGameplayEffectHandle& Handle : GameplayEffectHandles)
	{
		AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
	}

	AbilitySpecHandles.Reset();
	GameplayEffectHandles.Reset();
}

void UAtlasAbilitySet::GiveToAbilitySystem(UAtlasAbilitySystemComponent* AbilitySystemComponent, UObject* SourceObject,
	FAtlasAbilitySet_GrantedHandles* OutGrantedHandles) const
{
	if (AbilitySystemComponent == nullptr || !AbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return;
	}

	for (const FAtlasAbilitySet_GameplayAbility& AbilityToGrant : GrantedAbilities)
	{
		if (AbilityToGrant.Ability == nullptr)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(AbilityToGrant.Ability, AbilityToGrant.AbilityLevel, INDEX_NONE, SourceObject);
		if (AbilityToGrant.InputTag.IsValid())
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilityToGrant.InputTag);
		}

		const FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
		if (OutGrantedHandles != nullptr)
		{
			OutGrantedHandles->AddAbilitySpecHandle(SpecHandle);
		}
	}

	for (const FAtlasAbilitySet_GameplayEffect& EffectToGrant : GrantedEffects)
	{
		if (EffectToGrant.GameplayEffect == nullptr)
		{
			continue;
		}

		const FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		const FGameplayEffectSpecHandle EffectSpec = AbilitySystemComponent->MakeOutgoingSpec(
			EffectToGrant.GameplayEffect, EffectToGrant.EffectLevel, EffectContext);

		if (!EffectSpec.IsValid())
		{
			continue;
		}

		const FActiveGameplayEffectHandle EffectHandle =
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
		if (OutGrantedHandles != nullptr)
		{
			OutGrantedHandles->AddGameplayEffectHandle(EffectHandle);
		}
	}
}
