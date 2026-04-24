// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/GameplayAbility.h"
#include "Combat/AtlasCombatTypes.h"
#include "AtlasAbilityBase.generated.h"

class UAtlasCombatAttributeSet;
class AAtlasPlayerCombatCharacter;
class AAtlasCombatCharacterBase;
class UAtlasAbilitySystemComponent;

/**
 * 
 */
UCLASS(Abstract)
class UNICOMBAT_API UAtlasAbilityBase : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UAtlasAbilityBase();
	UFUNCTION(BlueprintPure, Category = "Ability")
	AAtlasCombatCharacterBase* GetCombatCharacterFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	UAtlasAbilitySystemComponent* GetAtlasAbilitySystemComponentFromActorInfo() const;
	
	UFUNCTION(BlueprintPure, Category = "Ability")
	FGameplayTag GetInputTag() const { return InputTag; }
	
	UFUNCTION(BlueprintCallable, Category="Atlas|Ability")
	bool GetAtlasAbilityContext(
		AAtlasPlayerCombatCharacter*& PlayerCharacter,
		UAtlasAbilitySystemComponent*& AtlasASC,
		UAtlasCombatAttributeSet*& Attributes
	);
	
	//判断是否在O附近
	UFUNCTION(BlueprintCallable, Category = "Atlas|Ability")
	bool AttributeDataIsZero(const FGameplayAttributeData InData);
	
	//句柄无效化
	UFUNCTION(BlueprintCallable, Category="Atlas|Ability")
	static void InvalidateActiveGameplayEffectHandle(UPARAM(ref) FActiveGameplayEffectHandle& Handle);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	EAtlasAbilityActivationPolicy ActivationPolicy = EAtlasAbilityActivationPolicy::OnInputTriggered;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (Categories = "Input"))
	FGameplayTag InputTag;
};
