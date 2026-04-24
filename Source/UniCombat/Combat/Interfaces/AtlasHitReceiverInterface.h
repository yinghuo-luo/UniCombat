// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/AtlasCombatTypes.h"
#include "UObject/Interface.h"
#include "AtlasHitReceiverInterface.generated.h"

UINTERFACE(BlueprintType)
class UNICOMBAT_API UAtlasHitReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

class UNICOMBAT_API IAtlasHitReceiverInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void HandleGameplayHit(const FAtlasCombatHitData& HitData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	void HandleGameplayDeath(const FAtlasCombatHitData& HitData);

	//可以被打击中断
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool CanBeInterruptedByHit(const FAtlasCombatHitData& HitData) const;
};
