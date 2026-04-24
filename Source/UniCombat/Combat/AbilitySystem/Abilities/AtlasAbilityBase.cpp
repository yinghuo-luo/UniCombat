// Fill out your copyright notice in the Description page of Project Settings.


#include "AtlasAbilityBase.h"
#include "Combat/AtlasGameplayTags.h"
#include "Combat/AbilitySystem/AtlasAbilitySystemComponent.h"
#include "Combat/Characters/AtlasCombatCharacterBase.h"
#include "Combat/Characters/AtlasPlayerCombatCharacter.h"

UAtlasAbilityBase::UAtlasAbilityBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationBlockedTags.AddTag(TAG_State_Dead);
}


AAtlasCombatCharacterBase* UAtlasAbilityBase::GetCombatCharacterFromActorInfo() const
{
	return Cast<AAtlasCombatCharacterBase>(GetAvatarActorFromActorInfo());
}

UAtlasAbilitySystemComponent* UAtlasAbilityBase::GetAtlasAbilitySystemComponentFromActorInfo() const
{
	return Cast<UAtlasAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
}

bool UAtlasAbilityBase::GetAtlasAbilityContext(
	AAtlasPlayerCombatCharacter*& PlayerCharacter, UAtlasAbilitySystemComponent*& AtlasASC,
	UAtlasCombatAttributeSet*& Attributes)
{
	PlayerCharacter = nullptr;
	AtlasASC = nullptr;
	Attributes = nullptr;
	
	PlayerCharacter = Cast<AAtlasPlayerCombatCharacter>(GetCombatCharacterFromActorInfo());
	AtlasASC = GetAtlasAbilitySystemComponentFromActorInfo();

	Attributes = (PlayerCharacter != nullptr)
		? const_cast<UAtlasCombatAttributeSet*>(PlayerCharacter->GetCombatAttributes())
		: nullptr;

	return PlayerCharacter != nullptr && AtlasASC != nullptr && Attributes != nullptr;
}

bool UAtlasAbilityBase::AttributeDataIsZero(const FGameplayAttributeData InData)
{
	if (InData.GetCurrentValue() <= KINDA_SMALL_NUMBER || InData.GetCurrentValue()>= KINDA_SMALL_NUMBER)
	{
		return true;
	}
	return false;
}

void UAtlasAbilityBase::InvalidateActiveGameplayEffectHandle(FActiveGameplayEffectHandle& Handle)
{
	Handle.Invalidate();
}


