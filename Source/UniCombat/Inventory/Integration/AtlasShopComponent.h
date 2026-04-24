#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/Runtime/AtlasInventoryTypes.h"
#include "AtlasShopComponent.generated.h"

class AActor;
class UAtlasInventorySubsystem;
class UAtlasItemDefinition;

USTRUCT(BlueprintType)
struct FAtlasShopEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TObjectPtr<UAtlasItemDefinition> ItemDefinition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop", meta = (ClampMin = "-1"))
	int32 PriceOverride = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop", meta = (ClampMin = "-1"))
	int32 Stock = -1;
};

UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasShopComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasShopComponent();

	UFUNCTION(BlueprintPure, Category = "Inventory|Shop")
	TArray<FAtlasShopEntry> GetEntries() const { return Entries; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Shop")
	bool BuyItem(AActor* Buyer, UAtlasItemDefinition* ItemDefinition, int32 Quantity, FAtlasItemHandle& OutHandle, FText& OutFailureReason);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Shop")
	bool SellItem(AActor* Seller, const FAtlasItemHandle& ItemHandle, int32 Quantity, int32& OutMoneyEarned, FText& OutFailureReason);

protected:
	UAtlasInventorySubsystem* ResolveInventorySubsystem(const AActor* BuyerOrSeller) const;
	int32 FindEntryIndex(const UAtlasItemDefinition* ItemDefinition) const;
	int32 ResolvePrice(const FAtlasShopEntry& Entry) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shop")
	TArray<FAtlasShopEntry> Entries;
};
