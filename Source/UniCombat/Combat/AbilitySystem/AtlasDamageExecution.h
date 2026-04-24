// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "AtlasDamageExecution.generated.h"

/*
 * @Execution 执行
 */
UCLASS()
class UNICOMBAT_API UAtlasDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UAtlasDamageExecution();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
