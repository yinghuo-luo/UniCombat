// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AtlasGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FAtlasGeneralParam
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool bMainQuestAlreadyClaimed = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool HasTutorial; //是否新手教程
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	bool UnlockedDifficulty; //已解锁难度
};

/**
 * 
 */
UCLASS()
class UNICOMBAT_API UAtlasGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;
	
	void ImportPersistenceSnapshot(const FAtlasGeneralParam& Snapshot);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category="AtlasGameInstance")
	FAtlasGeneralParam GeneralParam;
protected:
	UFUNCTION(blueprintNativeEvent,Category = "Atlas|Framework|GameInstance")
	void PostSubsystemInit();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Atlas|Framework|GameInstance")
	bool bShouldCreateNewSlot = false;
};
