// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/AtlasCombatTypes.h"
#include "UObject/Interface.h"
#include "AtlasRevealableTargetInterface.generated.h"

/*
 * @Revealable 可公开的
 */
UINTERFACE(BlueprintType)
class UNICOMBAT_API UAtlasRevealableTargetInterface : public UInterface
{
	GENERATED_BODY()
};

class UNICOMBAT_API IAtlasRevealableTargetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|Reveal")
	bool CanBeRevealed(const FAtlasRevealContext& RevealContext) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|Reveal")
	void HandleRevealTriggered(const FAtlasRevealContext& RevealContext);

	//Response响应 Reveal透露/表明/展示
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|Reveal")
	FGameplayTagContainer GetRevealResponseTags() const;
};
