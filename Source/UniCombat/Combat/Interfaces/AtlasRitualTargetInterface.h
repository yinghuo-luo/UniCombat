// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/AtlasCombatTypes.h"
#include "UObject/Interface.h"
#include "AtlasRitualTargetInterface.generated.h"

/*
 * Ritual 仪式
 */
UINTERFACE(BlueprintType)
class UNICOMBAT_API UAtlasRitualTargetInterface : public UInterface
{
	GENERATED_BODY()
};

class UNICOMBAT_API IAtlasRitualTargetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|Ritual")
	bool CanAcceptRitualStep(FGameplayTag RitualStepTag, const FAtlasRitualContext& RitualContext) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|Ritual")
	void HandleRitualStepApplied(FGameplayTag RitualStepTag, const FAtlasRitualContext& RitualContext);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|Ritual")
	FGameplayTagContainer GetCompletedRitualStepTags() const;
};
