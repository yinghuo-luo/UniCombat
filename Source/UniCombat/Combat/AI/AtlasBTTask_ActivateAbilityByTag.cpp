// Copyright Epic Games, Inc. All Rights Reserved.

#include "Combat/AI/AtlasBTTask_ActivateAbilityByTag.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Combat/Characters/AtlasEnemyCombatCharacter.h"

UAtlasBTTask_ActivateAbilityByTag::UAtlasBTTask_ActivateAbilityByTag()
{
	NodeName = TEXT("Activate Ability By Tag");
}

EBTNodeResult::Type UAtlasBTTask_ActivateAbilityByTag::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AAtlasEnemyCombatCharacter* EnemyCharacter = AIController != nullptr
		? Cast<AAtlasEnemyCombatCharacter>(AIController->GetPawn())
		: nullptr;

	if (EnemyCharacter == nullptr || !AbilityTag.IsValid())
	{
		//失败结束
		return EBTNodeResult::Failed;
	}

	return EnemyCharacter->TryUseCombatAbilityByTag(AbilityTag)
		? EBTNodeResult::Succeeded
		: EBTNodeResult::Failed;
}
