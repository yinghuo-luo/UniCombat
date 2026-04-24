// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/AtlasCombatTypes.h"
#include "Engine/DataAsset.h"
#include "AtlasEnemyCombatConfigData.generated.h"

UCLASS(BlueprintType, Const)
class UNICOMBAT_API UAtlasEnemyCombatConfigData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	//Aggro Range
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AggroRange = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float AttackRange = 250.0f;

	//Chase 追逐
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	float ChaseRange = 1600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TArray<FAtlasEnemyAbilityEntry> AbilityEntries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Boss")
	int32 InitialPhase = 0;
};
