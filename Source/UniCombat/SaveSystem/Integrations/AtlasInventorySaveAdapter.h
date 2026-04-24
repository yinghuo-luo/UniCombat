#pragma once

#include "CoreMinimal.h"
#include "AtlasInventorySaveGame.h"
#include "SaveSystem/Providers/AtlasSaveDataAdapterBase.h"
#include "AtlasInventorySaveAdapter.generated.h"

class UAtlasInventorySubsystem;

USTRUCT(BlueprintType)
struct FAtlasInventoryProviderPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FAtlasInventoryPersistenceSnapshot Snapshot;
};

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class UNICOMBAT_API UAtlasInventorySaveAdapter : public UAtlasSaveDataAdapterBase
{
	GENERATED_BODY()

public:
	virtual FName GetProviderId() const override;
	virtual int32 GetProviderVersion() const override;
	virtual FString GetDebugName() const override;
	virtual EAtlasSaveRestoreName GetRestoreName() const override;
	virtual bool GatherSaveData(const FAtlasSaveExecutionContext& Context, FAtlasProviderRecord& OutRecord) override;
	virtual bool ApplyLoadedData(const FAtlasLoadExecutionContext& Context, const FAtlasProviderRecord& Record) override;

protected:
	UAtlasInventorySubsystem* ResolveInventorySubsystem(UGameInstance* GameInstance) const;
};
