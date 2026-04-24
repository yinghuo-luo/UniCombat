// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/AtlasCombatTypes.h"
#include "UObject/Interface.h"
#include "AtlasExorcisableTargetInterface.generated.h"

/*
 * Exorcisable 可被驱魔的
 */
UINTERFACE(BlueprintType)
class UNICOMBAT_API UAtlasExorcisableTargetInterface : public UInterface
{
	GENERATED_BODY()
};

class UNICOMBAT_API IAtlasExorcisableTargetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|Exorcise")
	bool CanBeExorcised(const FAtlasExorciseContext& ExorciseContext) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|Exorcise")
	bool IsExorciseWindowOpen() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Occult|Exorcise")
	void HandleExorcised(const FAtlasExorciseContext& ExorciseContext);
};
