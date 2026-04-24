// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "QuestSystem/QuestTypes.h"
#include "QuestSystem/QuestEventTypes.h"
#include "QuestBlueprintLibrary.generated.h"

class UQuestSubsystem;

UCLASS()
class UNICOMBAT_API UQuestBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Quest", meta = (WorldContext = "WorldContextObject"))
	static UQuestSubsystem* GetQuestSubsystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (WorldContext = "WorldContextObject"))
	static bool AcceptQuest(const UObject* WorldContextObject, FName QuestId);

	UFUNCTION(BlueprintPure, Category = "Quest", meta = (WorldContext = "WorldContextObject"))
	static bool HasQuest(const UObject* WorldContextObject, FName QuestId);

	UFUNCTION(BlueprintPure, Category = "Quest", meta = (WorldContext = "WorldContextObject"))
	static EQuestState GetQuestState(const UObject* WorldContextObject, FName QuestId);

	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (WorldContext = "WorldContextObject"))
	static void SubmitQuestEvent(const UObject* WorldContextObject, const FQuestEventPayload& Payload);

	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (WorldContext = "WorldContextObject"))
	static void SubmitChoice(const UObject* WorldContextObject, FName QuestId, FName ChoiceKey, FName ChoiceResult);

	UFUNCTION(BlueprintPure, Category = "Quest", meta = (WorldContext = "WorldContextObject"))
	static bool GetWorldStateBool(const UObject* WorldContextObject, FName Key);

	UFUNCTION(BlueprintPure, Category = "Quest", meta = (WorldContext = "WorldContextObject"))
	static int32 GetWorldStateInt(const UObject* WorldContextObject, FName Key);

	UFUNCTION(BlueprintCallable, Category = "Quest", meta = (WorldContext = "WorldContextObject"))
	static void AddQuestCounter(const UObject* WorldContextObject, FName QuestId, FName CounterId, int32 Delta);

};
