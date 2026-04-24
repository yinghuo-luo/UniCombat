// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "AtlasBTTask_ActivateAbilityByTag.generated.h"

UCLASS()
class UNICOMBAT_API UAtlasBTTask_ActivateAbilityByTag : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAtlasBTTask_ActivateAbilityByTag();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Ability", meta = (Categories = "Ability"))
	FGameplayTag AbilityTag;
};
