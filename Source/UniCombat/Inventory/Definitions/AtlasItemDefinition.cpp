#include "Inventory/Definitions/AtlasItemDefinition.h"

#include "Engine/AssetManager.h"

const UAtlasItemDefinition* AtlasResolveItemDefinition(const FAtlasItemDefinitionRef& DefinitionRef)
{
	if (!DefinitionRef.DefinitionAsset.IsNull())
	{
		return DefinitionRef.DefinitionAsset.LoadSynchronous();
	}

	if (DefinitionRef.PrimaryAssetId.IsValid())
	{
		if (UAssetManager* AssetManager = UAssetManager::GetIfInitialized())
		{
			const FSoftObjectPath AssetPath = AssetManager->GetPrimaryAssetPath(DefinitionRef.PrimaryAssetId);
			return Cast<UAtlasItemDefinition>(AssetPath.TryLoad());
		}
	}

	return nullptr;
}

UAtlasItemDefinition::UAtlasItemDefinition()
{
}

FPrimaryAssetId UAtlasItemDefinition::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("AtlasItem"), ItemId.IsNone() ? GetFName() : ItemId);
}

FAtlasItemDefinitionRef UAtlasItemDefinition::ToDefinitionRef() const
{
	FAtlasItemDefinitionRef Ref;
	Ref.PrimaryAssetId = GetPrimaryAssetId();
	Ref.DefinitionAsset = const_cast<UAtlasItemDefinition*>(this);
	Ref.ItemId = ItemId;
	return Ref;
}

bool UAtlasItemDefinition::CanUse_Implementation() const
{
	return false;
}

void UAtlasItemDefinition::ApplyUse_Implementation(FAtlasItemUseResult& OutResult) const
{
	OutResult = FAtlasItemUseResult();
	OutResult.bSuccess = false;
	OutResult.FailureReason = NSLOCTEXT("AtlasInventory", "ItemCannotBeUsed", "This item cannot be used.");
}

UAtlasHealthItemDefinition::UAtlasHealthItemDefinition()
{
	ItemType = EAtlasItemType::Consumable;
	MaxStack = 99;
}

bool UAtlasHealthItemDefinition::CanUse_Implementation() const
{
	return HealthDelta > 0;
}

void UAtlasHealthItemDefinition::ApplyUse_Implementation(FAtlasItemUseResult& OutResult) const
{
	OutResult = FAtlasItemUseResult();
	if (HealthDelta <= 0)
	{
		OutResult.bSuccess = false;
		OutResult.FailureReason = NSLOCTEXT("AtlasInventory", "InvalidHealthItem", "Health delta must be greater than zero.");
		return;
	}

	OutResult.bSuccess = true;
	OutResult.ConsumedQuantity = 1;
	OutResult.HealthDelta = HealthDelta;
	OutResult.Message = FText::Format(
		NSLOCTEXT("AtlasInventory", "HealthRecovered", "Restore {0} health."),
		FText::AsNumber(HealthDelta));
}
