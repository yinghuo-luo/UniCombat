// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveSystem/AtlasSaveTypes.h"
#include "AtlasSaveGame.generated.h"

/**
 * 实际保存的数据
 */
UCLASS(BlueprintType)
class UNICOMBAT_API UAtlasRunSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UAtlasRunSaveGame();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,SaveGame)
	FAtlasSaveHeader Header;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,SaveGame)
	TArray<FAtlasProviderRecord> ProviderRecords;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FAtlasObjectSaveRecord> ObjectRecords;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TMap<FName, bool> GlobalFlags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TMap<FString, FString> Metadata; //
};

UCLASS(BlueprintType)
class UNICOMBAT_API UAtlasSaveSlotIndexSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UAtlasSaveSlotIndexSaveGame();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FAtlasSaveSlotDescriptor> SlotDescriptors; //槽 描述符
};

