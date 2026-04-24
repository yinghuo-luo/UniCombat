// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/AtlasCombatTypes.h"
#include "UObject/Interface.h"
#include "AtlasTargetableInterface.generated.h"

UINTERFACE(BlueprintType)
class UNICOMBAT_API UAtlasTargetableInterface : public UInterface
{
	GENERATED_BODY()
};

class UNICOMBAT_API IAtlasTargetableInterface
{
	GENERATED_BODY()

public:
	//可锁定
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targeting")
	bool CanBeLockedOn() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targeting")
	FVector GetLockTargetLocation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targeting")
	FText GetTargetDisplayName() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Targeting")
	EAtlasTargetCategory GetTargetCategory() const;
};
