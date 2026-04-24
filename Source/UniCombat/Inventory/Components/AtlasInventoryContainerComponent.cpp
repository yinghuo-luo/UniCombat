#include "Inventory/Components/AtlasInventoryContainerComponent.h"

#include "Engine/GameInstance.h"
#include "Inventory/AtlasInventoryGameplayTags.h"
#include "Inventory/Definitions/AtlasItemDefinition.h"
#include "Inventory/Integration/AtlasInventorySubsystem.h"

namespace
{
int32 ResolveMaxStack(const FAtlasItemStack& Stack)
{
	if (const UAtlasItemDefinition* Definition = AtlasResolveItemDefinition(Stack.DefinitionRef))
	{
		return FMath::Max(1, Definition->MaxStack);
	}

	return 1;
}
}

UAtlasInventoryContainerComponent::UAtlasInventoryContainerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAtlasInventoryContainerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UAtlasInventorySubsystem* InventorySubsystem = GameInstance->GetSubsystem<UAtlasInventorySubsystem>())
		{
			InventorySubsystem->RegisterContainer(this);
		}
	}
}

void UAtlasInventoryContainerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (UAtlasInventorySubsystem* InventorySubsystem = GameInstance->GetSubsystem<UAtlasInventorySubsystem>())
		{
			InventorySubsystem->UnregisterContainer(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

TArray<FAtlasItemStack> UAtlasInventoryContainerComponent::GetStacks() const
{
	TArray<FAtlasItemStack> Stacks;
	Stacks.Reserve(Entries.Num());
	for (const FAtlasContainerEntry& Entry : Entries)
	{
		Stacks.Add(Entry.Stack);
	}
	return Stacks;
}

int32 UAtlasInventoryContainerComponent::GetItemCountById(const FName ItemId) const
{
	int32 Total = 0;
	for (const FAtlasContainerEntry& Entry : Entries)
	{
		if (Entry.Stack.DefinitionRef.ItemId == ItemId)
		{
			Total += Entry.Stack.Quantity;
		}
	}
	return Total;
}

bool UAtlasInventoryContainerComponent::ContainsItemId(const FName ItemId, const int32 MinimumQuantity) const
{
	return GetItemCountById(ItemId) >= MinimumQuantity;
}

void UAtlasInventoryContainerComponent::ClearContainer()
{
	Entries.Reset();
	NotifyChanged();
}

bool UAtlasInventoryContainerComponent::TryAddStack(const FAtlasItemStack& Stack, int32 Quantity, const int32 PreferredSlot, const bool bProjectionOnly, const FAtlasItemHandle* SourceHandle, FAtlasItemHandle& OutHandle, FText& OutFailureReason)
{
	(void)bProjectionOnly;
	(void)SourceHandle;

	if (!CanAcceptStack(Stack, Quantity > 0 ? Quantity : Stack.Quantity, PreferredSlot, false, OutFailureReason))
	{
		return false;
	}

	const int32 RequestedQuantity = Quantity > 0 ? Quantity : Stack.Quantity;
	int32 RemainingQuantity = RequestedQuantity;
	const int32 MaxStack = ResolveMaxStack(Stack);

	if (bAllowAutoStacking && MaxStack > 1)
	{
		for (FAtlasContainerEntry& Entry : Entries)
		{
			if (Entry.Stack.DefinitionRef.ItemId != Stack.DefinitionRef.ItemId || Entry.Stack.Quantity >= MaxStack)
			{
				continue;
			}

			const int32 AcceptedQuantity = FMath::Min(RemainingQuantity, MaxStack - Entry.Stack.Quantity);
			Entry.Stack.Quantity += AcceptedQuantity;
			RemainingQuantity -= AcceptedQuantity;
			OutHandle = MakeHandleFromEntry(Entry);
			if (RemainingQuantity <= 0)
			{
				NotifyChanged();
				return true;
			}
		}
	}

	FAtlasItemStack WorkingStack = Stack;
	while (RemainingQuantity > 0)
	{
		const int32 SlotIndex = ResolvePreferredSlot(PreferredSlot);
		if (SlotIndex == INDEX_NONE)
		{
			OutFailureReason = NSLOCTEXT("AtlasInventory", "NoFreeSlot", "No free slot is available.");
			return false;
		}

		FAtlasContainerEntry& NewEntry = Entries.AddDefaulted_GetRef();
		NewEntry.Stack = WorkingStack;
		NewEntry.Stack.Quantity = FMath::Min(RemainingQuantity, MaxStack);
		if (!NewEntry.Stack.InstanceGuid.IsValid())
		{
			NewEntry.Stack.InstanceGuid = FGuid::NewGuid();
		}
		NewEntry.SlotIndex = SlotIndex;

		OutHandle = MakeHandleFromEntry(NewEntry);
		RemainingQuantity -= NewEntry.Stack.Quantity;
		WorkingStack.InstanceGuid = FGuid::NewGuid();
	}

	NotifyChanged();
	return true;
}

bool UAtlasInventoryContainerComponent::TryRemoveStack(const FAtlasItemHandle& Handle, const int32 Quantity, FAtlasItemStack& OutRemovedStack, FText& OutFailureReason)
{
	if (!Handle.IsValid())
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "InvalidHandle", "Invalid item handle.");
		return false;
	}

	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		FAtlasContainerEntry& Entry = Entries[Index];
		if (Entry.Stack.InstanceGuid != Handle.InstanceGuid || Entry.SlotIndex != Handle.SlotIndex)
		{
			continue;
		}

		const int32 RemoveQuantity = Quantity > 0 ? FMath::Min(Quantity, Entry.Stack.Quantity) : Entry.Stack.Quantity;
		OutRemovedStack = Entry.Stack;
		OutRemovedStack.Quantity = RemoveQuantity;
		Entry.Stack.Quantity -= RemoveQuantity;
		if (Entry.Stack.Quantity <= 0)
		{
			Entries.RemoveAt(Index);
		}

		NotifyChanged();
		return true;
	}

	OutFailureReason = NSLOCTEXT("AtlasInventory", "MissingItem", "Item was not found in this container.");
	return false;
}

bool UAtlasInventoryContainerComponent::FindEntryByHandle(const FAtlasItemHandle& Handle, FAtlasContainerEntry& OutEntry) const
{
	for (const FAtlasContainerEntry& Entry : Entries)
	{
		if (Entry.Stack.InstanceGuid == Handle.InstanceGuid && Entry.SlotIndex == Handle.SlotIndex)
		{
			OutEntry = Entry;
			return true;
		}
	}

	return false;
}

bool UAtlasInventoryContainerComponent::ReplaceEntryStack(const FAtlasItemHandle& Handle, const FAtlasItemStack& NewStack)
{
	for (FAtlasContainerEntry& Entry : Entries)
	{
		if (Entry.Stack.InstanceGuid == Handle.InstanceGuid && Entry.SlotIndex == Handle.SlotIndex)
		{
			Entry.Stack = NewStack;
			NotifyChanged();
			return true;
		}
	}

	return false;
}

void UAtlasInventoryContainerComponent::LoadEntries(const TArray<FAtlasContainerEntry>& InEntries)
{
	Entries = InEntries;
	for (FAtlasContainerEntry& Entry : Entries)
	{
		if (!Entry.Stack.InstanceGuid.IsValid())
		{
			Entry.Stack.InstanceGuid = FGuid::NewGuid();
		}
	}
	NotifyChanged();
}

FAtlasItemHandle UAtlasInventoryContainerComponent::MakeHandleFromEntry(const FAtlasContainerEntry& Entry) const
{
	FAtlasItemHandle Handle;
	Handle.InstanceGuid = Entry.Stack.InstanceGuid;
	Handle.ContainerType = ContainerType;
	Handle.ContainerTag = ContainerTag;
	Handle.SlotIndex = Entry.SlotIndex;
	return Handle;
}

bool UAtlasInventoryContainerComponent::CanAcceptStack(const FAtlasItemStack& Stack, const int32 Quantity, const int32 PreferredSlot, const bool bProjectionOnly, FText& OutFailureReason) const
{
	(void)bProjectionOnly;

	if (!Stack.IsValid())
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "InvalidStack", "Invalid item stack.");
		return false;
	}

	if (Quantity <= 0)
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "InvalidQuantity", "Quantity must be greater than zero.");
		return false;
	}

	if (!AtlasResolveItemDefinition(Stack.DefinitionRef))
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "MissingDefinition", "Item definition is missing.");
		return false;
	}

	int32 RemainingQuantity = Quantity;
	const int32 MaxStack = ResolveMaxStack(Stack);
	if (bAllowAutoStacking && MaxStack > 1)
	{
		for (const FAtlasContainerEntry& Entry : Entries)
		{
			if (Entry.Stack.DefinitionRef.ItemId != Stack.DefinitionRef.ItemId || Entry.Stack.Quantity >= MaxStack)
			{
				continue;
			}

			RemainingQuantity -= FMath::Min(RemainingQuantity, MaxStack - Entry.Stack.Quantity);
			if (RemainingQuantity <= 0)
			{
				return true;
			}
		}
	}

	if (PreferredSlot != INDEX_NONE && (PreferredSlot < 0 || PreferredSlot >= Capacity))
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "InvalidSlot", "Requested slot is outside the container capacity.");
		return false;
	}

	const int32 NeededSlots = FMath::DivideAndRoundUp(RemainingQuantity, MaxStack);
	if (NeededSlots > CountFreeSlots())
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "ContainerFull", "Container is full.");
		return false;
	}

	return true;
}

int32 UAtlasInventoryContainerComponent::FindFirstFreeSlot() const
{
	for (int32 SlotIndex = 0; SlotIndex < Capacity; ++SlotIndex)
	{
		const bool bOccupied = Entries.ContainsByPredicate([SlotIndex](const FAtlasContainerEntry& Entry)
		{
			return Entry.SlotIndex == SlotIndex;
		});
		if (!bOccupied)
		{
			return SlotIndex;
		}
	}

	return INDEX_NONE;
}

int32 UAtlasInventoryContainerComponent::CountFreeSlots() const
{
	int32 FreeSlots = 0;
	for (int32 SlotIndex = 0; SlotIndex < Capacity; ++SlotIndex)
	{
		const bool bOccupied = Entries.ContainsByPredicate([SlotIndex](const FAtlasContainerEntry& Entry)
		{
			return Entry.SlotIndex == SlotIndex;
		});
		if (!bOccupied)
		{
			++FreeSlots;
		}
	}
	return FreeSlots;
}

int32 UAtlasInventoryContainerComponent::ResolvePreferredSlot(const int32 PreferredSlot) const
{
	if (PreferredSlot != INDEX_NONE)
	{
		const bool bOccupied = Entries.ContainsByPredicate([PreferredSlot](const FAtlasContainerEntry& Entry)
		{
			return Entry.SlotIndex == PreferredSlot;
		});
		if (!bOccupied && PreferredSlot >= 0 && PreferredSlot < Capacity)
		{
			return PreferredSlot;
		}
	}

	return FindFirstFreeSlot();
}

void UAtlasInventoryContainerComponent::NotifyChanged()
{
	OnContainerChanged.Broadcast(this);
}

UAtlasBackpackComponent::UAtlasBackpackComponent()
{
	ContainerType = EAtlasContainerType::Backpack;
	ContainerTag = TAG_Inventory_Container_Player_Backpack;
	Capacity = 36;
}

UAtlasEquipmentComponent::UAtlasEquipmentComponent()
{
	ContainerType = EAtlasContainerType::Equipment;
	ContainerTag = TAG_Inventory_Container_Player_Equipment;
	Capacity = 8;
	bUseFixedSlots = true;
	bAllowAutoStacking = false;
}

UAtlasQuickSlotComponent::UAtlasQuickSlotComponent()
{
	ContainerType = EAtlasContainerType::QuickSlot;
	ContainerTag = TAG_Inventory_Container_Player_QuickSlot;
	Capacity = 6;
	bUseFixedSlots = true;
	bAllowAutoStacking = false;
}

UAtlasSafehouseStorageComponent::UAtlasSafehouseStorageComponent()
{
	ContainerType = EAtlasContainerType::SafehouseStorage;
	ContainerTag = TAG_Inventory_Container_Player_Storage;
	Capacity = 64;
}

UAtlasInvestigationBoardComponent::UAtlasInvestigationBoardComponent()
{
	ContainerType = EAtlasContainerType::InvestigationBoard;
	ContainerTag = TAG_Inventory_Container_Player_InvestigationBoard;
	Capacity = 6;
	bUseFixedSlots = true;
	bAllowAutoStacking = false;
}

UAtlasRitualLoadoutComponent::UAtlasRitualLoadoutComponent()
{
	ContainerType = EAtlasContainerType::RitualLoadout;
	ContainerTag = TAG_Inventory_Container_Player_RitualLoadout;
	Capacity = 8;
	bUseFixedSlots = true;
	bAllowAutoStacking = false;
}
