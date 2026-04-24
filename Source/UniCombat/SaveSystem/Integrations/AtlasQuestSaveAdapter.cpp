// Fill out your copyright notice in the Description page of Project Settings.


#include "AtlasQuestSaveAdapter.h"

FName UAtlasQuestSaveAdapter::GetProviderId() const
{
	static const FName ProviderId(TEXT("QuestSystem"));
	return ProviderId;
}

int32 UAtlasQuestSaveAdapter::GetProviderVersion() const
{
	return 1;
}

FString UAtlasQuestSaveAdapter::GetDebugName() const
{
	return TEXT("Quest Save Adapter");
}

EAtlasSaveRestoreName UAtlasQuestSaveAdapter::GetRestoreName() const
{
	return EAtlasSaveRestoreName::QuestSystem;
}

bool UAtlasQuestSaveAdapter::GatherSaveData(const FAtlasSaveExecutionContext& Context, FAtlasProviderRecord& OutRecord)
{
	UQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem(Context.GameInstance);
	if (!QuestSubsystem)
		return false;
	FAtlasQuestProviderPayload Payload;
	QuestSubsystem->ExportPersistenceSnapshot(Payload.Snapshot);
	return WriteTypedPayloadRecord(Payload, OutRecord);
}

bool UAtlasQuestSaveAdapter::ApplyLoadedData(const FAtlasLoadExecutionContext& Context,
	const FAtlasProviderRecord& Record)
{
	UQuestSubsystem* QuestSubsystem = ResolveQuestSubsystem(Context.GameInstance);
	if (!QuestSubsystem)
		return false;
	
	FAtlasQuestProviderPayload Payload;
	if (!ReadTypedPayloadRecord(Record, Payload))
	{
		return false;
	}

	QuestSubsystem->ImportPersistenceSnapshot(Payload.Snapshot);
	return true;
}

UQuestSubsystem* UAtlasQuestSaveAdapter::ResolveQuestSubsystem(UGameInstance* GameInstance) const
{
	return GameInstance ? GameInstance->GetSubsystem<UQuestSubsystem>() : nullptr;
}
