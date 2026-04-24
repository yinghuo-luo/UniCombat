// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Characters/AtlasCombatCharacterBase.h"
#include "AtlasEnemyCombatCharacter.generated.h"

class UAtlasEnemyCombatConfigData;

UCLASS()
class UNICOMBAT_API AAtlasEnemyCombatCharacter : public AAtlasCombatCharacterBase
{
	GENERATED_BODY()

public:
	//尝试通过标签使用战斗能力
	UFUNCTION(BlueprintCallable, Category = "Combat|AI")
	bool TryUseCombatAbilityByTag(FGameplayTag AbilityTag);

	UFUNCTION(BlueprintCallable, Category = "Combat|AI")
	bool TryUseConfiguredAbilityForDistance(float DistanceToTarget, FGameplayTag& OutChosenAbilityTag);

	UFUNCTION(BlueprintPure, Category = "Combat|AI")
	const UAtlasEnemyCombatConfigData* GetEnemyCombatConfig() const { return EnemyCombatConfig; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UAtlasEnemyCombatConfigData> EnemyCombatConfig = nullptr;
};
