// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestSystem/QuestTypes.h"
#include "QuestSystem/QuestEventTypes.h"
#include "QuestConditionEvaluator.generated.h"

class UQuestSubsystem;

/**
 * 
 */
UCLASS()
class UNICOMBAT_API UQuestConditionEvaluator : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(UQuestSubsystem* InSubsystem);
	
	/*
	 * @Param OutFailureReason 失败原因
	 */
	bool EvaluateConditionList(
		const TArray<FName>& ConditionIds,
		const FQuestRuntimeState& RuntimeState,
		const FQuestEventPayload* EventPayload,
		FString* OutFailureReason = nullptr) const;

	bool EvaluateCondition(
		const FQuestConditionRow& Condition,
		const FQuestRuntimeState& RuntimeState,
		const FQuestEventPayload* EventPayload,
		FString* OutFailureReason = nullptr) const;

private:
	bool EvaluateIntComparison(int32 LeftValue, EQuestCompareOp CompareOp, int32 RightValue) const;
	bool EvaluateBoolComparison(bool bLeftValue, EQuestCompareOp CompareOp, bool bRightValue) const;
	bool EvaluateNameComparison(FName LeftValue, EQuestCompareOp CompareOp, FName RightValue) const;

private:
	TWeakObjectPtr<UQuestSubsystem> QuestSubsystem;
};
