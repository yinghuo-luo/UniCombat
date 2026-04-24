// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AtlasCombatBlueprintLibrary.generated.h"

class AActor;
class UAbilitySystemComponent;
class UGameplayEffect;

UCLASS()
class UNICOMBAT_API UAtlasCombatBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	//从Actor获取能力系统
	UFUNCTION(BlueprintPure, Category = "Combat")
	static UAbilitySystemComponent* GetAbilitySystemFromActor(const AActor* Actor);

	//Actor是否敌对
	UFUNCTION(BlueprintPure, Category = "Combat")
	static bool AreActorsHostile(const AActor* SourceActor, const AActor* TargetActor);

	//应用伤害效果
	UFUNCTION(BlueprintCallable, Category = "Combat")
	static bool ApplyDamageEffect(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC,
		TSubclassOf<UGameplayEffect> DamageEffectClass, float AbilityLevel, float BaseDamage, float BasePoiseDamage,
		FGameplayTag DamageTypeTag, const FHitResult& HitResult, AActor* EffectCauser = nullptr,
		const FGameplayTagContainer& AdditionalAssetTags = FGameplayTagContainer());

	//将游戏效果应用于目标
	static bool ApplyGameplayEffectToTarget(UAbilitySystemComponent* SourceASC, AActor* TargetActor,
		TSubclassOf<UGameplayEffect> EffectClass, float AbilityLevel,
		const FGameplayTagContainer& DynamicAssetTags = FGameplayTagContainer());

	static bool ApplyGameplayEffectToSelf(UAbilitySystemComponent* AbilitySystemComponent,
		TSubclassOf<UGameplayEffect> EffectClass, float AbilityLevel,
		const FGameplayTagContainer& DynamicAssetTags = FGameplayTagContainer());

	//查找接口实现者
	static UObject* FindInterfaceImplementer(const AActor* Actor, UClass* InterfaceClass);

	//从Actor获取拥有的游戏标签
	static void GetOwnedGameplayTagsFromActor(const AActor* Actor, FGameplayTagContainer& OutTags);

	//Actor匹配的GameplayTag
	static bool ActorHasMatchingGameplayTag(const AActor* Actor, FGameplayTag GameplayTag);
};
