// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestBlueprintLibrary.h"

#include "Engine/GameInstance.h"
#include "QuestSubsystem.h"

UQuestSubsystem* UQuestBlueprintLibrary::GetQuestSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	if (UWorld* World = WorldContextObject->GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UQuestSubsystem>();
		}
	}

	return nullptr;
}

bool UQuestBlueprintLibrary::AcceptQuest(const UObject* WorldContextObject, const FName QuestId)
{
	if (UQuestSubsystem* QuestSubsystem = GetQuestSubsystem(WorldContextObject))
	{
		return QuestSubsystem->AcceptQuest(QuestId);
	}

	return false;
}

bool UQuestBlueprintLibrary::HasQuest(const UObject* WorldContextObject, const FName QuestId)
{
	if (const UQuestSubsystem* QuestSubsystem = GetQuestSubsystem(WorldContextObject))
	{
		return QuestSubsystem->HasQuest(QuestId);
	}

	return false;
}

EQuestState UQuestBlueprintLibrary::GetQuestState(const UObject* WorldContextObject, const FName QuestId)
{
	if (const UQuestSubsystem* QuestSubsystem = GetQuestSubsystem(WorldContextObject))
	{
		return QuestSubsystem->GetQuestState(QuestId);
	}

	return EQuestState::Inactive;
}

void UQuestBlueprintLibrary::SubmitQuestEvent(const UObject* WorldContextObject, const FQuestEventPayload& Payload)
{
	if (UQuestSubsystem* QuestSubsystem = GetQuestSubsystem(WorldContextObject))
	{
		QuestSubsystem->SubmitQuestEvent(Payload);
	}
}

void UQuestBlueprintLibrary::SubmitChoice(const UObject* WorldContextObject, const FName QuestId, const FName ChoiceKey, const FName ChoiceResult)
{
	if (UQuestSubsystem* QuestSubsystem = GetQuestSubsystem(WorldContextObject))
	{
		QuestSubsystem->SubmitChoice(QuestId, ChoiceKey, ChoiceResult);
	}
}

bool UQuestBlueprintLibrary::GetWorldStateBool(const UObject* WorldContextObject, const FName Key)
{
	if (const UQuestSubsystem* QuestSubsystem = GetQuestSubsystem(WorldContextObject))
	{
		return QuestSubsystem->GetWorldStateBool(Key);
	}

	return false;
}

int32 UQuestBlueprintLibrary::GetWorldStateInt(const UObject* WorldContextObject, const FName Key)
{
	if (const UQuestSubsystem* QuestSubsystem = GetQuestSubsystem(WorldContextObject))
	{
		return QuestSubsystem->GetWorldStateInt(Key);
	}

	return 0;
}

void UQuestBlueprintLibrary::AddQuestCounter(const UObject* WorldContextObject, const FName QuestId, const FName CounterId, const int32 Delta)
{
	if (UQuestSubsystem* QuestSubsystem = GetQuestSubsystem(WorldContextObject))
	{
		QuestSubsystem->AddQuestCounter(QuestId, CounterId, Delta);
	}
}