#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/Runtime/AtlasInventoryTypes.h"
#include "AtlasInventorySaveGame.generated.h"

USTRUCT(BlueprintType)
struct FAtlasSavedItemRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FAtlasItemDefinitionRef DefinitionRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FGuid InstanceGuid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	EAtlasItemSourceType SourceType = EAtlasItemSourceType::Unknown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 SlotIndex = INDEX_NONE;
};

USTRUCT(BlueprintType)
struct FAtlasSavedContainerData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	EAtlasContainerType ContainerType = EAtlasContainerType::Backpack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FGameplayTag ContainerTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FAtlasSavedItemRecord> Items;
};

USTRUCT(BlueprintType)
struct FAtlasInventoryPersistenceSnapshot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 DataVersion = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 Money = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TArray<FAtlasSavedContainerData> Containers;
};

UCLASS()
class UNICOMBAT_API UAtlasInventorySaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FAtlasInventoryPersistenceSnapshot Snapshot;
};
