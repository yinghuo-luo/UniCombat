#include "Inventory/Integration/AtlasShopComponent.h"

#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"
#include "Inventory/Definitions/AtlasItemDefinition.h"
#include "Inventory/Integration/AtlasInventorySubsystem.h"

UAtlasShopComponent::UAtlasShopComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAtlasShopComponent::BuyItem(AActor* Buyer, UAtlasItemDefinition* ItemDefinition, const int32 Quantity, FAtlasItemHandle& OutHandle, FText& OutFailureReason)
{
	if (!Buyer || !ItemDefinition || Quantity <= 0)
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "InvalidShopPurchase", "Invalid purchase request.");
		return false;
	}

	UAtlasInventorySubsystem* InventorySubsystem = ResolveInventorySubsystem(Buyer);
	if (!InventorySubsystem)
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "MissingInventorySubsystem", "Inventory subsystem is missing.");
		return false;
	}

	const int32 EntryIndex = FindEntryIndex(ItemDefinition);
	if (EntryIndex == INDEX_NONE)
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "MissingShopEntry", "Item is not sold by this shop.");
		return false;
	}

	FAtlasShopEntry& Entry = Entries[EntryIndex];
	if (Entry.Stock >= 0 && Entry.Stock < Quantity)
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "InsufficientStock", "Shop stock is insufficient.");
		return false;
	}

	const int32 TotalPrice = ResolvePrice(Entry) * Quantity;
	if (!InventorySubsystem->TrySpendMoney(TotalPrice))
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "NotEnoughMoney", "Not enough money.");
		return false;
	}

	if (!InventorySubsystem->AddItem(ItemDefinition, Quantity, nullptr, EAtlasItemSourceType::ShopPurchase, FGameplayTag(), OutHandle))
	{
		InventorySubsystem->AddMoney(TotalPrice);
		OutFailureReason = NSLOCTEXT("AtlasInventory", "PurchaseFailed", "Failed to add item to inventory.");
		return false;
	}

	if (Entry.Stock >= 0)
	{
		Entry.Stock -= Quantity;
	}

	return true;
}

bool UAtlasShopComponent::SellItem(AActor* Seller, const FAtlasItemHandle& ItemHandle, const int32 Quantity, int32& OutMoneyEarned, FText& OutFailureReason)
{
	OutMoneyEarned = 0;
	if (!Seller || !ItemHandle.IsValid() || Quantity <= 0)
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "InvalidShopSale", "Invalid sale request.");
		return false;
	}

	UAtlasInventorySubsystem* InventorySubsystem = ResolveInventorySubsystem(Seller);
	if (!InventorySubsystem)
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "MissingInventorySubsystem", "Inventory subsystem is missing.");
		return false;
	}

	FAtlasItemStack Stack;
	if (!InventorySubsystem->GetItemStack(ItemHandle, Stack))
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "MissingSaleItem", "Item was not found.");
		return false;
	}

	if (Quantity > Stack.Quantity)
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "NotEnoughItemsToSell", "Not enough items to sell.");
		return false;
	}

	UAtlasItemDefinition* Definition = InventorySubsystem->GetItemDefinition(ItemHandle);
	if (!Definition || !Definition->bCanSell || Definition->SellPrice <= 0)
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "ItemCannotBeSold", "This item cannot be sold.");
		return false;
	}

	if (!InventorySubsystem->RemoveItem(ItemHandle, Quantity))
	{
		OutFailureReason = NSLOCTEXT("AtlasInventory", "SellRemoveFailed", "Failed to remove item from inventory.");
		return false;
	}

	OutMoneyEarned = Definition->SellPrice * Quantity;
	InventorySubsystem->AddMoney(OutMoneyEarned);

	const int32 EntryIndex = FindEntryIndex(Definition);
	if (EntryIndex != INDEX_NONE && Entries[EntryIndex].Stock >= 0)
	{
		Entries[EntryIndex].Stock += Quantity;
	}

	return true;
}

UAtlasInventorySubsystem* UAtlasShopComponent::ResolveInventorySubsystem(const AActor* BuyerOrSeller) const
{
	UGameInstance* GameInstance = BuyerOrSeller ? BuyerOrSeller->GetGameInstance() : nullptr;
	return GameInstance ? GameInstance->GetSubsystem<UAtlasInventorySubsystem>() : nullptr;
}

int32 UAtlasShopComponent::FindEntryIndex(const UAtlasItemDefinition* ItemDefinition) const
{
	if (!ItemDefinition)
	{
		return INDEX_NONE;
	}

	return Entries.IndexOfByPredicate([ItemDefinition](const FAtlasShopEntry& Entry)
	{
		return Entry.ItemDefinition == ItemDefinition
			|| (Entry.ItemDefinition && Entry.ItemDefinition->ItemId == ItemDefinition->ItemId);
	});
}

int32 UAtlasShopComponent::ResolvePrice(const FAtlasShopEntry& Entry) const
{
	if (Entry.PriceOverride >= 0)
	{
		return Entry.PriceOverride;
	}

	return Entry.ItemDefinition ? Entry.ItemDefinition->BuyPrice : 0;
}
