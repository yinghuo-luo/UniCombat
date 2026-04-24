// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AtlasSaveSystemSetting.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="SaveSystem"))
class UNICOMBAT_API UAtlasSaveSystemSetting : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Config, Category = "Save|Slots")
	int32 DefaultUserIndex = 0;
	UPROPERTY(EditAnywhere, Config, Category = "Save|Slots")
	int32 SlotIndexUserIndex = 0;
	UPROPERTY(EditAnywhere, Config, Category = "Save|Slots")
	FString SlotIndexSlotName = TEXT("MSD_SlotIndex");
	UPROPERTY(EditAnywhere, Config, Category = "Save|Slots")
	FString DefaultAutosaveSlot = TEXT("Auto_Default");
	//检查点槽前缀
	UPROPERTY(EditAnywhere, Config, Category = "Save|Slots")
	FString CheckpointSlotPrefix = TEXT("Checkpoint");
	
	UPROPERTY(EditAnywhere, Config, Category = "Save|Version", meta = (ClampMin = "1", UIMin = "1"))
	int32 CurrentSaveVersion = 1;
	UPROPERTY(EditAnywhere, Config, Category = "Save|Version", meta = (ClampMin = "1", UIMin = "1"))
	int32 MinimumLoadableSaveVersion = 1;
	//版本策略
	UPROPERTY(EditAnywhere, Config, Category = "Save|Version")
	FSoftClassPath VersionPolicyClass;
	//默认提供者类
	UPROPERTY(EditAnywhere, Config, Category = "Save|Providers")
	TArray<FSoftClassPath> DefaultProviderClasses;
};
