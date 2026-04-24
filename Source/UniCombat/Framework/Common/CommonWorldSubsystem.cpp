// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonWorldSubsystem.h"

#include "CommonTypes.h"
#include "CommonWorldSetting.h"
#include "Framework/AtlasGameInstance.h"
#include "QuestSystem/Core/QuestSubsystem.h"
#include "SaveSystem/Core/AtlasSaveBlueprintLibrary.h"

void UCommonWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (World->WorldType != EWorldType::Game &&
		World->WorldType != EWorldType::PIE)
	{
		return;
	}

	const AWorldSettings* WorldSettings = World->GetWorldSettings();
	CurrentWorldSettings = Cast<ACommonWorldSetting>(WorldSettings);
	if (!CurrentWorldSettings)
	{
		UE_LOG(LogTemp, Warning, TEXT("ACommonWorldSetting: WorldSettings is not ACommonWorldSetting, skip special logic."));
	}

	if (UAtlasGameInstance* GI = Cast<UAtlasGameInstance>(World->GetGameInstance()))
	{
		if (GI->bShouldCreateNewSlot)
		{
			UAtlasSaveBlueprintLibrary::SaveAutosave(this, AutoSaveName);
		}
	}

	const FAtlasRestoreBatchResult Res = UAtlasSaveBlueprintLibrary::ApplyProviderRecordByName(
		this, EAtlasSaveRestoreName::GeneralParam);
	if (!Res.bSuccess && !Res.FailureReason.IsEmpty()
		&& Res.FailureReason != TEXT("No loaded save is pending restore."))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to restore GeneralParam in CommonWorldSubsystem::Initialize: %s"),
			*Res.FailureReason);
		return;
	}

	if (UAtlasGameInstance* GI = Cast<UAtlasGameInstance>(World->GetGameInstance()))
	{
		if (CurrentWorldSettings
			&& CurrentWorldSettings->bCreateMainQuest
			&& !GI->GeneralParam.bMainQuestAlreadyClaimed)
		{
			if (UQuestSubsystem* QuestSubsystem = GI->GetSubsystem<UQuestSubsystem>())
			{
				if (QuestSubsystem->AcceptQuest(TEXT("Q_Main_VisitShrine")))
				{
					GI->GeneralParam.bMainQuestAlreadyClaimed = true;
				}
			}
		}
		else
		{
			//恢复任务
			RecoveryQuest(World);
		}
		
	}
}

void UCommonWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UCommonWorldSubsystem::RecoveryQuest(const UWorld* World)
{
	const FAtlasRestoreBatchResult Res = UAtlasSaveBlueprintLibrary::ApplyProviderRecordByName(
		this, EAtlasSaveRestoreName::QuestSystem);
	if (!Res.bSuccess && !Res.FailureReason.IsEmpty()
		&& Res.FailureReason != TEXT("No loaded save is pending restore."))
	{
		return;
	}

	if (UAtlasGameInstance* GI = Cast<UAtlasGameInstance>(World->GetGameInstance()))
	{
		if (UQuestSubsystem* QuestSubsystem = GI->GetSubsystem<UQuestSubsystem>())
		{
			
		}
	}
}
