// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/AtlasCombatTypes.h"
#include "UObject/Interface.h"
#include "AtlasSoulBellReactiveInterface.generated.h"

UINTERFACE(BlueprintType)
class UNICOMBAT_API UAtlasSoulBellReactiveInterface : public UInterface
{
	GENERATED_BODY()
};

class UNICOMBAT_API IAtlasSoulBellReactiveInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|SoulBell")
	bool CanReactToSoulBell(const FAtlasSoulBellContext& BellContext) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|SoulBell")
	void HandleSoulBellTriggered(const FAtlasSoulBellContext& BellContext);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|SoulBell")
	bool CanOpenExecutionWindowFromBell() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|SoulBell")
	FGameplayTagContainer GetSoulBellResponseTags() const;
};
