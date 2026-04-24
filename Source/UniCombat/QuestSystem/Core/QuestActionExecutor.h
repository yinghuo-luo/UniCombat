// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestSystem/QuestTypes.h"
#include "QuestSystem/QuestEventTypes.h"
#include "QuestActionExecutor.generated.h"

class UQuestSubsystem;
/**
 * 
 */
UCLASS()
class UNICOMBAT_API UQuestActionExecutor : public UObject
{
	GENERATED_BODY()
public:
	void Initialize(UQuestSubsystem* InSubsystem);

	void ExecuteActions(const FName QuestId, const TArray<FName>& ActionIds, const FQuestEventPayload& ContextPayload);
	void ExecuteAction(const FName QuestId, const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);

private:
	bool ShouldDispatchActionToTarget(const FQuestActionRow& ActionRow) const;


	TWeakObjectPtr<UQuestSubsystem> QuestSubsystem;
};
