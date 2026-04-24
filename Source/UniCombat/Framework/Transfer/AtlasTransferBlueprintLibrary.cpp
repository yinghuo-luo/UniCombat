#include "Framework/Transfer/AtlasTransferBlueprintLibrary.h"

#include "Framework/Transfer/AtlasTransferSubsystem.h"

UAtlasTransferSubsystem* UAtlasTransferBlueprintLibrary::GetTransferSubsystem(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UAtlasTransferSubsystem>();
		}
	}

	return nullptr;
}

bool UAtlasTransferBlueprintLibrary::StartTransfer(const UObject* WorldContextObject, const FAtlasTransferActionRequest& Request)
{
	if (UAtlasTransferSubsystem* TransferSubsystem = GetTransferSubsystem(WorldContextObject))
	{
		return TransferSubsystem->StartTransfer(Request);
	}

	return false;
}
