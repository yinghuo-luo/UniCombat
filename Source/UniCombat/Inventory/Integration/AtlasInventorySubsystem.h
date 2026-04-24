#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Inventory/Runtime/AtlasInventoryTypes.h"
#include "SaveSystem/Integrations/AtlasInventorySaveGame.h"
#include "AtlasInventorySubsystem.generated.h"

class AActor;
class UAtlasInventoryContainerComponent;
class UAtlasItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAtlasInventoryItemChangedEvent, FAtlasItemHandle, Handle, FAtlasItemStack, Stack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAtlasInventoryContainerEvent, UAtlasInventoryContainerComponent*, Container);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAtlasInventoryMoneyChangedEvent, int32, NewMoney);

USTRUCT(BlueprintType)
struct FAtlasInventoryTransferRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAtlasInventoryContainerComponent> FromContainer = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAtlasInventoryContainerComponent> ToContainer = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAtlasItemHandle ItemHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetSlot = INDEX_NONE;
};

UCLASS(BlueprintType)
class UNICOMBAT_API UAtlasInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Containers")
	void RegisterContainer(UAtlasInventoryContainerComponent* Container);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Containers")
	void UnregisterContainer(UAtlasInventoryContainerComponent* Container);

	UFUNCTION(BlueprintPure, Category = "Inventory|Containers")
	UAtlasInventoryContainerComponent* FindContainerByTag(FGameplayTag ContainerTag) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Containers")
	TArray<UAtlasInventoryContainerComponent*> GetContainersOfType(EAtlasContainerType ContainerType) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Containers")
	UAtlasInventoryContainerComponent* GetFirstContainerOfType(EAtlasContainerType ContainerType) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	bool AddItem(UAtlasItemDefinition* Definition, int32 Quantity, UAtlasInventoryContainerComponent* TargetContainer, EAtlasItemSourceType SourceType, FGameplayTag IgnoredQuestTag, FAtlasItemHandle& OutHandle);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	bool RemoveItem(const FAtlasItemHandle& ItemHandle, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	bool ConsumeItem(const FAtlasItemHandle& ItemHandle, int32 Quantity, FAtlasItemUseResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	bool TryUseItem(const FAtlasItemHandle& ItemHandle, EAtlasUsageContext UsageContext, UObject* WorldContextObject, AActor* InstigatorActor, FAtlasItemUseResult& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Items")
	bool TransferItem(const FAtlasInventoryTransferRequest& Request, FText& OutFailureReason);

	UFUNCTION(BlueprintPure, Category = "Inventory|Query")
	bool GetItemStack(const FAtlasItemHandle& ItemHandle, FAtlasItemStack& OutStack) const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Query")
	UAtlasItemDefinition* GetItemDefinition(const FAtlasItemHandle& ItemHandle) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
	void SaveToRecord(UAtlasInventoryContainerComponent* Container, FAtlasSavedContainerData& OutRecord) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory|Save")
	void LoadFromRecord(const FAtlasSavedContainerData& Record);

	void ExportPersistenceSnapshot(FAtlasInventoryPersistenceSnapshot& OutSnapshot) const;
	void ImportPersistenceSnapshot(const FAtlasInventoryPersistenceSnapshot& Snapshot);

	UFUNCTION(BlueprintPure, Category = "Inventory|Money")
	int32 GetMoney() const { return Money; }

	UFUNCTION(BlueprintCallable, Category = "Inventory|Money")
	void AddMoney(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Money")
	bool TrySpendMoney(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Inventory|State")
	bool HasItem(FName ItemId, int32 MinimumQuantity = 1) const;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FAtlasInventoryItemChangedEvent OnItemAdded;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FAtlasInventoryItemChangedEvent OnItemRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FAtlasInventoryContainerEvent OnContainerUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FAtlasInventoryMoneyChangedEvent OnMoneyChanged;

protected:
	UFUNCTION()
	void HandleRegisteredContainerChanged(UAtlasInventoryContainerComponent* ChangedContainer);

	bool ResolveHandle(const FAtlasItemHandle& ItemHandle, UAtlasInventoryContainerComponent*& OutContainer, FAtlasContainerEntry& OutEntry) const;
	FAtlasItemStack BuildRuntimeStack(UAtlasItemDefinition* Definition, int32 Quantity, EAtlasItemSourceType SourceType) const;
	void CleanupDeadContainerRefs();

protected:
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<UAtlasInventoryContainerComponent>> RegisteredContainers;

	UPROPERTY(Transient)
	int32 Money = 0;

	TMap<FGameplayTag, FAtlasSavedContainerData> PendingContainerRestoreData;
};
