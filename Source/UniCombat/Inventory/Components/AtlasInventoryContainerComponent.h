#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/Runtime/AtlasInventoryTypes.h"
#include "AtlasInventoryContainerComponent.generated.h"

class UAtlasItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAtlasOnContainerChanged, UAtlasInventoryContainerComponent*, Container);

UCLASS(Blueprintable, ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasInventoryContainerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasInventoryContainerComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure, Category = "Inventory|Container")
	EAtlasContainerType GetContainerType() const { return ContainerType; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Container")
	FGameplayTag GetContainerTag() const { return ContainerTag; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Container")
	int32 GetCapacity() const { return Capacity; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Container")
	bool UsesFixedSlots() const { return bUseFixedSlots; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Container")
	TArray<FAtlasContainerEntry> GetEntries() const { return Entries; }

	UFUNCTION(BlueprintPure, Category = "Inventory|Container")
	TArray<FAtlasItemStack> GetStacks() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Container")
	int32 GetItemCountById(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Container")
	bool ContainsItemId(FName ItemId, int32 MinimumQuantity = 1) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Container")
	void ClearContainer();

	bool TryAddStack(const FAtlasItemStack& Stack, int32 Quantity, int32 PreferredSlot, bool bProjectionOnly, const FAtlasItemHandle* SourceHandle, FAtlasItemHandle& OutHandle, FText& OutFailureReason);
	bool TryRemoveStack(const FAtlasItemHandle& Handle, int32 Quantity, FAtlasItemStack& OutRemovedStack, FText& OutFailureReason);
	bool FindEntryByHandle(const FAtlasItemHandle& Handle, FAtlasContainerEntry& OutEntry) const;
	bool ReplaceEntryStack(const FAtlasItemHandle& Handle, const FAtlasItemStack& NewStack);
	void LoadEntries(const TArray<FAtlasContainerEntry>& InEntries);
	FAtlasItemHandle MakeHandleFromEntry(const FAtlasContainerEntry& Entry) const;
	bool CanAcceptStack(const FAtlasItemStack& Stack, int32 Quantity, int32 PreferredSlot, bool bProjectionOnly, FText& OutFailureReason) const;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Container")
	FAtlasOnContainerChanged OnContainerChanged;

protected:
	int32 FindFirstFreeSlot() const;
	int32 CountFreeSlots() const;
	int32 ResolvePreferredSlot(int32 PreferredSlot) const;
	void NotifyChanged();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Container")
	EAtlasContainerType ContainerType = EAtlasContainerType::Backpack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Container")
	FGameplayTag ContainerTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Container", meta = (ClampMin = "1"))
	int32 Capacity = 32;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Container")
	bool bUseFixedSlots = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Container")
	bool bAllowAutoStacking = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Container")
	TArray<FAtlasContainerEntry> Entries;
};

UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasBackpackComponent : public UAtlasInventoryContainerComponent
{
	GENERATED_BODY()

public:
	UAtlasBackpackComponent();
};

UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasEquipmentComponent : public UAtlasInventoryContainerComponent
{
	GENERATED_BODY()

public:
	UAtlasEquipmentComponent();
};

UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasQuickSlotComponent : public UAtlasInventoryContainerComponent
{
	GENERATED_BODY()

public:
	UAtlasQuickSlotComponent();
};

UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasSafehouseStorageComponent : public UAtlasInventoryContainerComponent
{
	GENERATED_BODY()

public:
	UAtlasSafehouseStorageComponent();
};

UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasInvestigationBoardComponent : public UAtlasInventoryContainerComponent
{
	GENERATED_BODY()

public:
	UAtlasInvestigationBoardComponent();
};

UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasRitualLoadoutComponent : public UAtlasInventoryContainerComponent
{
	GENERATED_BODY()

public:
	UAtlasRitualLoadoutComponent();
};
