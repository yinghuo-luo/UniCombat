#include "AtlasInventorySaveAdapter.h"

#include "Engine/GameInstance.h"
#include "Inventory/Integration/AtlasInventorySubsystem.h"

FName UAtlasInventorySaveAdapter::GetProviderId() const
{
	static const FName ProviderId(TEXT("InventoryFramework"));
	return ProviderId;
}

int32 UAtlasInventorySaveAdapter::GetProviderVersion() const
{
	return 2;
}

FString UAtlasInventorySaveAdapter::GetDebugName() const
{
	return TEXT("Inventory Save Adapter");
}

EAtlasSaveRestoreName UAtlasInventorySaveAdapter::GetRestoreName() const
{
	return EAtlasSaveRestoreName::Inventory;
}

bool UAtlasInventorySaveAdapter::GatherSaveData(const FAtlasSaveExecutionContext& Context, FAtlasProviderRecord& OutRecord)
{
	UAtlasInventorySubsystem* InventorySubsystem = ResolveInventorySubsystem(Context.GameInstance);
	if (!InventorySubsystem)
	{
		return false;
	}

	FAtlasInventoryProviderPayload Payload;
	InventorySubsystem->ExportPersistenceSnapshot(Payload.Snapshot);
	return WriteTypedPayloadRecord(Payload, OutRecord);
}

bool UAtlasInventorySaveAdapter::ApplyLoadedData(const FAtlasLoadExecutionContext& Context, const FAtlasProviderRecord& Record)
{
	UAtlasInventorySubsystem* InventorySubsystem = ResolveInventorySubsystem(Context.GameInstance);
	if (!InventorySubsystem)
	{
		return false;
	}

	FAtlasInventoryProviderPayload Payload;
	if (!ReadTypedPayloadRecord(Record, Payload))
	{
		return false;
	}

	InventorySubsystem->ImportPersistenceSnapshot(Payload.Snapshot);
	return true;
}

UAtlasInventorySubsystem* UAtlasInventorySaveAdapter::ResolveInventorySubsystem(UGameInstance* GameInstance) const
{
	return GameInstance ? GameInstance->GetSubsystem<UAtlasInventorySubsystem>() : nullptr;
}
