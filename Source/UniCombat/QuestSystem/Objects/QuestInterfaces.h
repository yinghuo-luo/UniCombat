// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestSystem/QuestEventTypes.h"
#include "QuestSystem/QuestTypes.h"
#include "QuestInterfaces.generated.h"

UINTERFACE(BlueprintType)
class UNICOMBAT_API UQuestEventSourceInterface : public UInterface
{
	GENERATED_BODY()
};

class UNICOMBAT_API IQuestEventSourceInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	FName GetQuestEventSourceId() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	void BuildQuestEventPayload(EQuestEventType EventType, FQuestEventPayload& OutPayload) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	void NotifyQuestEventSubmitted(const FQuestEventPayload& Payload);
};

UINTERFACE(BlueprintType)
class UNICOMBAT_API UQuestActionTargetInterface : public UInterface
{
	GENERATED_BODY()
};

class UNICOMBAT_API IQuestActionTargetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	FName GetQuestTargetId() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	void SetQuestTargetEnabled(bool bEnabled);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	bool IsQuestTargetEnabled() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	void ApplyQuestAction(const FQuestActionRow& ActionRow, const FQuestEventPayload& ContextPayload);
};

