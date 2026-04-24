// Fill out your copyright notice in the Description page of Project Settings.


#include "AtlasSaveBlueprintLibrary.h"

#include "AtlasSaveSubsystem.h"

UAtlasSaveSubsystem* UAtlasSaveBlueprintLibrary::GetSaveSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
		return nullptr;
	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UAtlasSaveSubsystem>();
		}
	}
	return nullptr;
}

bool UAtlasSaveBlueprintLibrary::SavaManual(const UObject* WorldContextObject, const FString& SlotName,
	const FString& DisplayName)
{
	if (UAtlasSaveSubsystem* SaveSubsystem = GetSaveSubsystem(WorldContextObject))
	{
		return SaveSubsystem->SaveManual(SlotName,DisplayName);
	}
	return false;
}

bool UAtlasSaveBlueprintLibrary::SaveAutosave(const UObject* WorldContextObject, FName ContextId)
{
	if (UAtlasSaveSubsystem* SaveSubsystem = GetSaveSubsystem(WorldContextObject))
	{
		return SaveSubsystem->SaveAutosave(ContextId);
	}

	return false;
}

bool UAtlasSaveBlueprintLibrary::SaveCheckpoint(const UObject* WorldContextObject, FName CheckpointId)
{
	if (UAtlasSaveSubsystem* SaveSubsystem = GetSaveSubsystem(WorldContextObject))
	{
		return SaveSubsystem->SaveCheckpoint(CheckpointId);
	}

	return false;
}

bool UAtlasSaveBlueprintLibrary::LoadSlot(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex)
{
	if (UAtlasSaveSubsystem* SaveSubsystem = GetSaveSubsystem(WorldContextObject))
	{
		FAtlasLoadRequest Request;
		Request.SlotName = SlotName;
		Request.UserIndex = UserIndex;
		return SaveSubsystem->Load_SAV(Request);
	}
	return false;
}

bool UAtlasSaveBlueprintLibrary::DeleteSlot(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex)
{
	if (UAtlasSaveSubsystem* SaveSubsystem = GetSaveSubsystem(WorldContextObject))
	{
		return SaveSubsystem->DeleteSlot(SlotName, UserIndex);
	}

	return false;
}

TArray<FAtlasSaveSlotDescriptor> UAtlasSaveBlueprintLibrary::EnumerateSlots(const UObject* WorldContextObject)
{
	if (UAtlasSaveSubsystem* SaveSubsystem = GetSaveSubsystem(WorldContextObject))
	{
		return SaveSubsystem->EnumerateSlots();
	}

	return TArray<FAtlasSaveSlotDescriptor>();
}

FAtlasRestoreBatchResult UAtlasSaveBlueprintLibrary::ApplyProviderRecordByName(const UObject* WorldContextObject,
	EAtlasSaveRestoreName Name)
{
	if (UAtlasSaveSubsystem* SaveSubsystem = GetSaveSubsystem(WorldContextObject))
	{
		return SaveSubsystem->ApplyProviderRecords(Name);
	}

	FAtlasRestoreBatchResult Result;
	Result.RestoreName = Name;
	Result.FailureReason = TEXT("Save subsystem is unavailable.");
	return Result;
}

FAtlasRestoreBatchResult UAtlasSaveBlueprintLibrary::ApplyObjectRecordsByName(const UObject* WorldContextObject, EAtlasSaveRestoreName Name)
{
	if (UAtlasSaveSubsystem* SaveSubsystem = GetSaveSubsystem(WorldContextObject))
	{
		return SaveSubsystem->ApplyObjectRecords(Name);
	}

	FAtlasRestoreBatchResult Result;
	Result.RestoreName = Name;
	Result.FailureReason = TEXT("Save subsystem is unavailable.");
	return Result;
}
