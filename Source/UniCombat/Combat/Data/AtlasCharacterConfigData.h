// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/AtlasCombatTypes.h"
#include "Combat/AbilitySystem/Abilities/AtlasAbilityBase.h"
#include "Engine/DataAsset.h"
#include "AtlasCharacterConfigData.generated.h"

class UGameplayEffect;
class UAtlasAbilitySet;

UCLASS(BlueprintType, Const)
class UNICOMBAT_API UAtlasCharacterConfigData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	EAtlasCombatFaction DefaultFaction = EAtlasCombatFaction::Enemy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	EAtlasTargetCategory TargetCategory = EAtlasTargetCategory::Normal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FGameplayTagContainer DefaultUnitTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TObjectPtr<UAtlasAbilitySet> DefaultAbilitySet = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TSubclassOf<UGameplayEffect> InitialAttributeEffect; //初始属性效应
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	TArray<FGameplayTag> StartupAbilityTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float AbilityGrantLevel = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting")
	FName LockSocketName = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Targeting")
	FVector LockOffset = FVector(0.0f, 0.0f, 80.0f);
};
