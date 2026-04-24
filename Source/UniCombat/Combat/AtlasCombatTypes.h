// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AtlasCombatTypes.generated.h"

class AActor;

/*
 * @Faction 阵营/派系
 */

UENUM(BlueprintType)
enum class EAtlasCombatFaction : uint8
{
	Player,
	Enemy,
	Neutral, //中立
	Summon //召唤
};

/*
 * @Category 类别
 */
UENUM(BlueprintType)
enum class EAtlasTargetCategory : uint8
{
	Normal, //普通
	Elite, //精英
	Boss, //首领
	Ghost, //幽灵
	Corpse, //尸体
	Summon, //召唤
	Device //装置
};

UENUM(BlueprintType)
enum class EAtlasAbilityActivationPolicy : uint8
{
	OnInputTriggered, //输入触发时
	WhileInputHeld, //按住输入时
	OnGranted, //授权时
	Passive //被动
};

USTRUCT(BlueprintType)
struct FAtlasCombatHitData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	//@Causer 起因者
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> EffectCauser = nullptr; 

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FHitResult HitResult;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTag DamageTypeTag;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTagContainer SourceTags;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	FGameplayTagContainer TargetTags;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float Damage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float PoiseDamage = 0.0f;

	//致命的
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bIsFatal = false;
};

/*
 * Entry入口
 */
USTRUCT(BlueprintType)
struct FAtlasEnemyAbilityEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	FGameplayTag AbilityTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float MinRange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float MaxRange = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float Weight = 1.0f;
};

/*
 * Reveal 展示
 */
USTRUCT(BlueprintType)
struct FAtlasRevealContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	FGameplayTag SourceAbilityTag;

	//显示窗口持续时间
	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	float RevealWindowDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	bool bCombatReveal = false;
};

USTRUCT(BlueprintType)
struct FAtlasSoulBellContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	FGameplayTag SourceAbilityTag;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	float EffectRadius = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	float StabilizeDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	bool bRequestsExecutionWindow = false;
};

USTRUCT(BlueprintType)
struct FAtlasExorciseContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	FGameplayTag SourceAbilityTag;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	bool bForceKill = false;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	bool bIgnoreWindowRequirement = false;
};

/*
 * Ritual 仪式
 */
USTRUCT(BlueprintType)
struct FAtlasRitualContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	TObjectPtr<AActor> InstigatorActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	FGameplayTag SourceAbilityTag;

	UPROPERTY(BlueprintReadOnly, Category = "Occult")
	float RitualStrength = 1.0f;
};
