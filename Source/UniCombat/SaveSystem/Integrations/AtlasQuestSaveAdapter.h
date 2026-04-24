// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestSystem/QuestTypes.h"
#include "QuestSystem/Core/QuestSubsystem.h"
#include "SaveSystem/Providers/AtlasSaveDataAdapterBase.h"
#include "AtlasQuestSaveAdapter.generated.h"

USTRUCT(BlueprintType)
struct FAtlasQuestProviderPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FQuestPersistenceSnapshot Snapshot;
};
/**
 * 
 */
UCLASS()
class UNICOMBAT_API UAtlasQuestSaveAdapter : public UAtlasSaveDataAdapterBase
{
	GENERATED_BODY()
public:
	virtual FName GetProviderId() const override;
	virtual int32 GetProviderVersion() const override;
	virtual FString GetDebugName() const override;
	virtual EAtlasSaveRestoreName GetRestoreName() const override;
	
	/*
	 * @Gather 收集
	 */
	virtual bool GatherSaveData(const FAtlasSaveExecutionContext& Context, FAtlasProviderRecord& OutRecord) override;
	virtual bool ApplyLoadedData(const FAtlasLoadExecutionContext& Context, const FAtlasProviderRecord& Record) override;
protected:
	/*
	 *@ Resolve 解决 /
	 */
	UQuestSubsystem* ResolveQuestSubsystem(UGameInstance* GameInstance) const;
};
