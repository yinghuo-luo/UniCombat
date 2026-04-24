#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/PrimaryAssetId.h"
#include "AtlasInventoryTypes.generated.h"

class UAtlasItemDefinition;

UENUM(BlueprintType)
enum class EAtlasItemType : uint8
{
	Quest,
	Consumable,
	Material,
	Equipment
};

UENUM(BlueprintType)
enum class EAtlasContainerType : uint8
{
	Backpack,
	Equipment,
	QuickSlot,
	SafehouseStorage,
	InvestigationBoard,
	RitualLoadout
};

UENUM(BlueprintType)
enum class EAtlasUsageContext : uint8
{
	Field,
	Safehouse
};

UENUM(BlueprintType)
enum class EAtlasItemSourceType : uint8
{
	Unknown,
	WorldPickup,
	ShopPurchase,
	ShopSell
};

USTRUCT(BlueprintType)
struct FAtlasItemDefinitionRef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FPrimaryAssetId PrimaryAssetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	TSoftObjectPtr<UAtlasItemDefinition> DefinitionAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FName ItemId = NAME_None;

	bool IsValid() const
	{
		return !ItemId.IsNone() || PrimaryAssetId.IsValid() || !DefinitionAsset.IsNull();
	}
};

USTRUCT(BlueprintType)
struct FAtlasItemHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid InstanceGuid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAtlasContainerType ContainerType = EAtlasContainerType::Backpack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ContainerTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = INDEX_NONE;

	bool IsValid() const
	{
		return InstanceGuid.IsValid() && SlotIndex != INDEX_NONE;
	}

	friend bool operator==(const FAtlasItemHandle& Left, const FAtlasItemHandle& Right)
	{
		return Left.InstanceGuid == Right.InstanceGuid
			&& Left.ContainerTag == Right.ContainerTag
			&& Left.SlotIndex == Right.SlotIndex;
	}
};

FORCEINLINE uint32 GetTypeHash(const FAtlasItemHandle& Handle)
{
	return HashCombine(GetTypeHash(Handle.InstanceGuid), HashCombine(GetTypeHash(Handle.ContainerTag), GetTypeHash(Handle.SlotIndex)));
}

USTRUCT(BlueprintType)
struct FAtlasItemStack
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

	bool IsValid() const
	{
		return DefinitionRef.IsValid() && Quantity > 0;
	}
};

USTRUCT(BlueprintType)
struct FAtlasItemUseResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText FailureReason;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ConsumedQuantity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HealthDelta = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Message;
};

USTRUCT(BlueprintType)
struct FAtlasContainerEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	FAtlasItemStack Stack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
	int32 SlotIndex = INDEX_NONE;
};
