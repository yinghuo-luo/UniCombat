#include "Inventory/Integration/AtlasInventorySubsystem.h"

#include "Inventory/Components/AtlasInventoryContainerComponent.h"
#include "Inventory/Definitions/AtlasItemDefinition.h"

void UAtlasInventorySubsystem::Deinitialize()
{
	for (const TWeakObjectPtr<UAtlasInventoryContainerComponent>& ContainerPtr : RegisteredContainers)
	{
		if (UAtlasInventoryContainerComponent* Container = ContainerPtr.Get())
		{
			Container->OnContainerChanged.RemoveDynamic(this, &ThisClass::HandleRegisteredContainerChanged);
		}
	}

	RegisteredContainers.Reset();
	PendingContainerRestoreData.Reset();
	Money = 0;
	Super::Deinitialize();
}

void UAtlasInventorySubsystem::RegisterContainer(UAtlasInventoryContainerComponent* Container)
{
	if (!IsValid(Container))
	{
		return;
	}

	CleanupDeadContainerRefs();
	const bool bAlreadyRegistered = RegisteredContainers.ContainsByPredicate([Container](const TWeakObjectPtr<UAtlasInventoryContainerComponent>& Existing)
	{
		return Existing.Get() == Container;
	});
	if (bAlreadyRegistered)
	{
		return;
	}

	RegisteredContainers.Add(Container);
	Container->OnContainerChanged.AddDynamic(this, &ThisClass::HandleRegisteredContainerChanged);

	if (const FAtlasSavedContainerData* PendingRecord = PendingContainerRestoreData.Find(Container->GetContainerTag()))
	{
		LoadFromRecord(*PendingRecord);
		PendingContainerRestoreData.Remove(Container->GetContainerTag());
	}

	HandleRegisteredContainerChanged(Container);
}

void UAtlasInventorySubsystem::UnregisterContainer(UAtlasInventoryContainerComponent* Container)
{
	if (!IsValid(Container))
	{
		return;
	}

	Container->OnContainerChanged.RemoveDynamic(this, &ThisClass::HandleRegisteredContainerChanged);
	RegisteredContainers.RemoveAll([Container](const TWeakObjectPtr<UAtlasInventoryContainerComponent>& Existing)
	{
		return Existing.Get() == Container;
	});
}

UAtlasInventoryContainerComponent* UAtlasInventorySubsystem::FindContainerByTag(const FGameplayTag ContainerTag) const
{
	for (const TWeakObjectPtr<UAtlasInventoryContainerComponent>& ContainerPtr : RegisteredContainers)
	{
		if (UAtlasInventoryContainerComponent* Container = ContainerPtr.Get())
		{
			if (Container->GetContainerTag() == ContainerTag)
			{
				return Container;
			}
		}
	}

	return nullptr;
}

TArray<UAtlasInventoryContainerComponent*> UAtlasInventorySubsystem::GetContainersOfType(const EAtlasContainerType ContainerType) const
{
	TArray<UAtlasInventoryContainerComponent*> Containers;
	for (const TWeakObjectPtr<UAtlasInventoryContainerComponent>& ContainerPtr : RegisteredContainers)
	{
		if (UAtlasInventoryContainerComponent* Container = ContainerPtr.Get())
		{
			if (Container->GetContainerType() == ContainerType)
			{
				Containers.Add(Container);
			}
		}
	}
	return Containers;
}

UAtlasInventoryContainerComponent* UAtlasInventorySubsystem::GetFirstContainerOfType(const EAtlasContainerType ContainerType) const
{
	for (const TWeakObjectPtr<UAtlasInventoryContainerComponent>& ContainerPtr : RegisteredContainers)
	{
		if (UAtlasInventoryContainerComponent* Container = ContainerPtr.Get())
		{
			if (Container->GetContainerType() == ContainerType)
			{
				return Container;
			}
		}
	}

	return nullptr;
}

bool UAtlasInventorySubsystem::AddItem(UAtlasItemDefinition* Definition, const int32 Quantity, UAtlasInventoryContainerComponent* TargetContainer, const EAtlasItemSourceType SourceType, const FGameplayTag IgnoredQuestTag, FAtlasItemHandle& OutHandle)
{
	(void)IgnoredQuestTag;

	if (!Definition || Quantity <= 0)
	{
		return false;
	}

	if (!TargetContainer)
	{
		TargetContainer = GetFirstContainerOfType(EAtlasContainerType::Backpack);
	}
	if (!TargetContainer)
	{
		return false;
	}

	const FAtlasItemStack RuntimeStack = BuildRuntimeStack(Definition, Quantity, SourceType);
	FText FailureReason;
	if (!TargetContainer->TryAddStack(RuntimeStack, Quantity, INDEX_NONE, false, nullptr, OutHandle, FailureReason))
	{
		return false;
	}

	OnItemAdded.Broadcast(OutHandle, RuntimeStack);
	return true;
}

bool UAtlasInventorySubsystem::RemoveItem(const FAtlasItemHandle& ItemHandle, const int32 Quantity)
{
	UAtlasInventoryContainerComponent* Container = nullptr;
	FAtlasContainerEntry Entry;
	if (!ResolveHandle(ItemHandle, Container, Entry) || !Container)
	{
		return false;
	}

	FAtlasItemStack RemovedStack;
	FText FailureReason;
	if (!Container->TryRemoveStack(ItemHandle, Quantity, RemovedStack, FailureReason))
	{
		return false;
	}

	OnItemRemoved.Broadcast(ItemHandle, RemovedStack);
	return true;
}

bool UAtlasInventorySubsystem::ConsumeItem(const FAtlasItemHandle& ItemHandle, const int32 Quantity, FAtlasItemUseResult& OutResult)
{
	if (!RemoveItem(ItemHandle, Quantity))
	{
		OutResult = FAtlasItemUseResult();
		OutResult.FailureReason = NSLOCTEXT("AtlasInventory", "ConsumeFailed", "Failed to consume item.");
		return false;
	}

	OutResult = FAtlasItemUseResult();
	OutResult.bSuccess = true;
	OutResult.ConsumedQuantity = Quantity;
	return true;
}

bool UAtlasInventorySubsystem::TryUseItem(const FAtlasItemHandle& ItemHandle, const EAtlasUsageContext UsageContext, UObject* WorldContextObject, AActor* InstigatorActor, FAtlasItemUseResult& OutResult)
{
	(void)UsageContext;
	(void)WorldContextObject;
	(void)InstigatorActor;

	UAtlasInventoryContainerComponent* Container = nullptr;
	FAtlasContainerEntry Entry;
	if (!ResolveHandle(ItemHandle, Container, Entry))
	{
		OutResult = FAtlasItemUseResult();
		OutResult.FailureReason = NSLOCTEXT("AtlasInventory", "MissingUseItem", "Item could not be found.");
		return false;
	}

	const UAtlasItemDefinition* Definition = AtlasResolveItemDefinition(Entry.Stack.DefinitionRef);
	if (!Definition)
	{
		OutResult = FAtlasItemUseResult();
		OutResult.FailureReason = NSLOCTEXT("AtlasInventory", "MissingDefinition", "Item definition is missing.");
		return false;
	}

	if (!Definition->CanUse())
	{
		OutResult = FAtlasItemUseResult();
		OutResult.FailureReason = NSLOCTEXT("AtlasInventory", "CannotUseItem", "This item cannot be used.");
		return false;
	}

	Definition->ApplyUse(OutResult);
	if (!OutResult.bSuccess)
	{
		return false;
	}

	if (OutResult.ConsumedQuantity > 0 && !RemoveItem(ItemHandle, OutResult.ConsumedQuantity))
	{
		OutResult = FAtlasItemUseResult();
		OutResult.FailureReason = NSLOCTEXT("AtlasInventory", "ConsumeAfterUseFailed", "Item use succeeded but the stack could not be updated.");
		return false;
	}

	if (OutResult.Message.IsEmpty())
	{
		OutResult.Message = Definition->DisplayName;
	}

	return true;
}

bool UAtlasInventorySubsystem::TransferItem(const FAtlasInventoryTransferRequest& Request, FText& OutFailureReason)
{
	UAtlasInventoryContainerComponent* SourceContainer = Request.FromContainer;
	FAtlasContainerEntry SourceEntry;
	if (!ResolveHandle(Request.ItemHandle, SourceContainer, SourceEntry) || !SourceContainer)
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "TransferSourceMissing", "Transfer source could not be resolved.");
		return false;
	}

	if (!Request.ToContainer)
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "TransferTargetMissing", "Transfer target is missing.");
		return false;
	}

	if (!Request.ToContainer->CanAcceptStack(SourceEntry.Stack, Request.Quantity, Request.TargetSlot, false, OutFailureReason))
	{
		return false;
	}

	FAtlasItemStack RemovedStack;
	if (!SourceContainer->TryRemoveStack(Request.ItemHandle, Request.Quantity, RemovedStack, OutFailureReason))
	{
		return false;
	}

	FAtlasItemHandle AddedHandle;
	if (!Request.ToContainer->TryAddStack(RemovedStack, RemovedStack.Quantity, Request.TargetSlot, false, nullptr, AddedHandle, OutFailureReason))
	{
		FAtlasItemHandle RollbackHandle;
		FText RollbackReason;
		SourceContainer->TryAddStack(RemovedStack, RemovedStack.Quantity, Request.ItemHandle.SlotIndex, false, nullptr, RollbackHandle, RollbackReason);
		return false;
	}

	return true;
}

bool UAtlasInventorySubsystem::GetItemStack(const FAtlasItemHandle& ItemHandle, FAtlasItemStack& OutStack) const
{
	UAtlasInventoryContainerComponent* Container = nullptr;
	FAtlasContainerEntry Entry;
	if (!ResolveHandle(ItemHandle, Container, Entry))
	{
		return false;
	}

	OutStack = Entry.Stack;
	return true;
}

UAtlasItemDefinition* UAtlasInventorySubsystem::GetItemDefinition(const FAtlasItemHandle& ItemHandle) const
{
	FAtlasItemStack Stack;
	return GetItemStack(ItemHandle, Stack) ? const_cast<UAtlasItemDefinition*>(AtlasResolveItemDefinition(Stack.DefinitionRef)) : nullptr;
}

void UAtlasInventorySubsystem::SaveToRecord(UAtlasInventoryContainerComponent* Container, FAtlasSavedContainerData& OutRecord) const
{
	if (!Container)
	{
		return;
	}

	OutRecord.ContainerType = Container->GetContainerType();
	OutRecord.ContainerTag = Container->GetContainerTag();
	OutRecord.Items.Reset();

	for (const FAtlasContainerEntry& Entry : Container->GetEntries())
	{
		FAtlasSavedItemRecord& SavedItem = OutRecord.Items.AddDefaulted_GetRef();
		SavedItem.DefinitionRef = Entry.Stack.DefinitionRef;
		SavedItem.Quantity = Entry.Stack.Quantity;
		SavedItem.InstanceGuid = Entry.Stack.InstanceGuid;
		SavedItem.SourceType = Entry.Stack.SourceType;
		SavedItem.SlotIndex = Entry.SlotIndex;
	}
}

void UAtlasInventorySubsystem::LoadFromRecord(const FAtlasSavedContainerData& Record)
{
	if (UAtlasInventoryContainerComponent* Container = FindContainerByTag(Record.ContainerTag))
	{
		TArray<FAtlasContainerEntry> LoadedEntries;
		for (const FAtlasSavedItemRecord& SavedItem : Record.Items)
		{
			FAtlasContainerEntry& Entry = LoadedEntries.AddDefaulted_GetRef();
			Entry.Stack.DefinitionRef = SavedItem.DefinitionRef;
			Entry.Stack.Quantity = SavedItem.Quantity;
			Entry.Stack.InstanceGuid = SavedItem.InstanceGuid;
			Entry.Stack.SourceType = SavedItem.SourceType;
			Entry.SlotIndex = SavedItem.SlotIndex;
		}

		Container->LoadEntries(LoadedEntries);
		return;
	}

	PendingContainerRestoreData.Add(Record.ContainerTag, Record);
}

void UAtlasInventorySubsystem::ExportPersistenceSnapshot(FAtlasInventoryPersistenceSnapshot& OutSnapshot) const
{
	OutSnapshot.DataVersion = 2;
	OutSnapshot.Money = Money;
	OutSnapshot.Containers.Reset();

	for (const TWeakObjectPtr<UAtlasInventoryContainerComponent>& ContainerPtr : RegisteredContainers)
	{
		if (UAtlasInventoryContainerComponent* Container = ContainerPtr.Get())
		{
			FAtlasSavedContainerData& Record = OutSnapshot.Containers.AddDefaulted_GetRef();
			SaveToRecord(Container, Record);
		}
	}
}

void UAtlasInventorySubsystem::ImportPersistenceSnapshot(const FAtlasInventoryPersistenceSnapshot& Snapshot)
{
	Money = Snapshot.Money;
	PendingContainerRestoreData.Reset();

	for (const FAtlasSavedContainerData& Record : Snapshot.Containers)
	{
		LoadFromRecord(Record);
	}

	OnMoneyChanged.Broadcast(Money);
}

void UAtlasInventorySubsystem::AddMoney(const int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	Money += Amount;
	OnMoneyChanged.Broadcast(Money);
}

bool UAtlasInventorySubsystem::TrySpendMoney(const int32 Amount)
{
	if (Amount <= 0)
	{
		return true;
	}

	if (Money < Amount)
	{
		return false;
	}

	Money -= Amount;
	OnMoneyChanged.Broadcast(Money);
	return true;
}

bool UAtlasInventorySubsystem::HasItem(const FName ItemId, const int32 MinimumQuantity) const
{
	int32 Total = 0;
	for (const TWeakObjectPtr<UAtlasInventoryContainerComponent>& ContainerPtr : RegisteredContainers)
	{
		if (const UAtlasInventoryContainerComponent* Container = ContainerPtr.Get())
		{
			Total += Container->GetItemCountById(ItemId);
			if (Total >= MinimumQuantity)
			{
				return true;
			}
		}
	}
	return false;
}

void UAtlasInventorySubsystem::HandleRegisteredContainerChanged(UAtlasInventoryContainerComponent* ChangedContainer)
{
	if (ChangedContainer)
	{
		OnContainerUpdated.Broadcast(ChangedContainer);
	}
}

bool UAtlasInventorySubsystem::ResolveHandle(const FAtlasItemHandle& ItemHandle, UAtlasInventoryContainerComponent*& OutContainer, FAtlasContainerEntry& OutEntry) const
{
	OutContainer = FindContainerByTag(ItemHandle.ContainerTag);
	if (OutContainer && OutContainer->FindEntryByHandle(ItemHandle, OutEntry))
	{
		return true;
	}

	for (const TWeakObjectPtr<UAtlasInventoryContainerComponent>& ContainerPtr : RegisteredContainers)
	{
		if (UAtlasInventoryContainerComponent* Container = ContainerPtr.Get())
		{
			if (Container->FindEntryByHandle(ItemHandle, OutEntry))
			{
				OutContainer = Container;
				return true;
			}
		}
	}

	return false;
}

FAtlasItemStack UAtlasInventorySubsystem::BuildRuntimeStack(UAtlasItemDefinition* Definition, const int32 Quantity, const EAtlasItemSourceType SourceType) const
{
	FAtlasItemStack Stack;
	Stack.DefinitionRef = Definition->ToDefinitionRef();
	Stack.Quantity = Quantity;
	Stack.InstanceGuid = FGuid::NewGuid();
	Stack.SourceType = SourceType;
	return Stack;
}

void UAtlasInventorySubsystem::CleanupDeadContainerRefs()
{
	RegisteredContainers.RemoveAll([](const TWeakObjectPtr<UAtlasInventoryContainerComponent>& ContainerPtr)
	{
		return !ContainerPtr.IsValid();
	});
}
