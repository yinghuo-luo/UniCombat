// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/AtlasCombatTypes.h"
#include "UObject/Interface.h"
#include "AtlasCombatantInterface.generated.h"

class UAbilitySystemComponent;

UINTERFACE(BlueprintType)
class UNICOMBAT_API UAtlasCombatantInterface : public UInterface
{
	GENERATED_BODY()
};

class UNICOMBAT_API IAtlasCombatantInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	EAtlasCombatFaction GetCombatFaction() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool IsAlive() const;

	//@Targetable 目标
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	bool IsCombatTargetable() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	UAbilitySystemComponent* GetCombatAbilitySystemComponent() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	FGameplayTagContainer GetOwnedCombatTags() const;

	//@Aim 目的/瞄准
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Combat")
	FVector GetCombatAimPoint(FName PointName) const;
};
