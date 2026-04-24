#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Inventory/Runtime/AtlasInventoryTypes.h"
#include "AtlasItemDefinition.generated.h"

class AActor;
class UStaticMesh;
class UTexture2D;

UNICOMBAT_API const UAtlasItemDefinition* AtlasResolveItemDefinition(const FAtlasItemDefinitionRef& DefinitionRef);

UCLASS(BlueprintType, Blueprintable)
class UNICOMBAT_API UAtlasItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UAtlasItemDefinition();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UFUNCTION(BlueprintPure, Category = "Inventory|Item")
	FAtlasItemDefinitionRef ToDefinitionRef() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory|Item")
	bool CanUse() const;
	virtual bool CanUse_Implementation() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Inventory|Item")
	void ApplyUse(FAtlasItemUseResult& OutResult) const;
	virtual void ApplyUse_Implementation(FAtlasItemUseResult& OutResult) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	EAtlasItemType ItemType = EAtlasItemType::Material;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "1"))
	int32 MaxStack = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0"))
	int32 BuyPrice = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = "0"))
	int32 SellPrice = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bCanSell = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int32 SortPriority = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSubclassOf<AActor> PickupActorClass;
};

UCLASS(BlueprintType, Blueprintable)
class UNICOMBAT_API UAtlasHealthItemDefinition : public UAtlasItemDefinition
{
	GENERATED_BODY()

public:
	UAtlasHealthItemDefinition();

	virtual bool CanUse_Implementation() const override;
	virtual void ApplyUse_Implementation(FAtlasItemUseResult& OutResult) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Use")
	int32 HealthDelta = 0;
};
