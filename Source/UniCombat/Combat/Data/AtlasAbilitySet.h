// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "AtlasAbilitySet.generated.h"

class UGameplayEffect;
class UAtlasAbilitySystemComponent;
class UGameplayAbility;

USTRUCT(BlueprintType)
struct FAtlasAbilitySet_GameplayAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayAbility> Ability;

	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	int32 AbilityLevel = 1;
	
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	bool IsShowIcon = false;
	
	UPROPERTY(EditDefaultsOnly, Category = "Abilities", meta = (Categories = "Input"))
	FGameplayTag InputTag;
};

USTRUCT(BlueprintType)
struct FAtlasAbilitySet_GameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> GameplayEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float EffectLevel = 1.0f;
};
/*
 * @Granted 已授予的
 */
USTRUCT(BlueprintType)
struct FAtlasAbilitySet_GrantedHandles
{
	GENERATED_BODY()

public:
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void AddGameplayEffectHandle(const FActiveGameplayEffectHandle& Handle);
	//Take 拿/从
	void TakeFromAbilitySystem(UAtlasAbilitySystemComponent* AbilitySystemComponent);

private:
	UPROPERTY(Transient)
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY(Transient)
	TArray<FActiveGameplayEffectHandle> GameplayEffectHandles;
};

/*
 * Const 这个 UObject 类在蓝图里应当被视为只读 / 常量对象。
UCLASS(..., Const) 会让这个类的对象在很多反射和蓝图场景下表现为“常量”语义，也就是：
蓝图里通常不应该修改它的成员
相关函数调用会更偏向只读访问
它表达的是“这类对象是配置数据，不是运行时可变状态”
这很适合 UPrimaryDataAsset 这种类型，因为 DataAsset 本来就常常拿来做：
静态配置
预设参数
技能表
数值表的面向对象封装
 */
UCLASS(BlueprintType, Const)
class UNICOMBAT_API UAtlasAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//授予的能力
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TArray<FAtlasAbilitySet_GameplayAbility> GrantedAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TArray<FAtlasAbilitySet_GameplayEffect> GrantedEffects;

	void GiveToAbilitySystem(UAtlasAbilitySystemComponent* AbilitySystemComponent, UObject* SourceObject,
		FAtlasAbilitySet_GrantedHandles* OutGrantedHandles = nullptr) const;
};
