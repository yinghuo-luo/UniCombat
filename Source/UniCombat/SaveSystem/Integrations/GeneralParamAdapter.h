// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveSystem/Providers/AtlasSaveDataAdapterBase.h"
#include "Framework/AtlasGameInstance.h"
#include "GeneralParamAdapter.generated.h"

USTRUCT(BlueprintType)
struct FAtlasGeneralParamPayload
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FAtlasGeneralParam Snapshot;
};
/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class UNICOMBAT_API UGeneralParamAdapter : public UAtlasSaveDataAdapterBase
{
	GENERATED_BODY()
public:
	virtual FName GetProviderId() const override
	{
		static const FName ProviderId(TEXT("GeneralGameParam"));
		return ProviderId;
	}
	virtual int32 GetProviderVersion() const override
	{
		return 1;
	}
	virtual FString GetDebugName() const override
	{
		return TEXT("General Param");
	}
	virtual EAtlasSaveRestoreName GetRestoreName() const override
	{
		return EAtlasSaveRestoreName::GeneralParam;
	}
	
	virtual bool GatherSaveData(const FAtlasSaveExecutionContext& Context, FAtlasProviderRecord& OutRecord) override;
	virtual bool ApplyLoadedData(const FAtlasLoadExecutionContext& Context, const FAtlasProviderRecord& Record) override;
	
};
