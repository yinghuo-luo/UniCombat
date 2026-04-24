#include "Inventory/Integration/AtlasItemPickupActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Inventory/Definitions/AtlasItemDefinition.h"
#include "Inventory/Integration/AtlasInventorySubsystem.h"

AAtlasItemPickupActor::AAtlasItemPickupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
}

bool AAtlasItemPickupActor::CollectPickup(AActor* Collector)
{
	if (!Collector || !ItemDefinition)
	{
		return false;
	}

	UGameInstance* GameInstance = Collector->GetGameInstance();
	UAtlasInventorySubsystem* InventorySubsystem = GameInstance ? GameInstance->GetSubsystem<UAtlasInventorySubsystem>() : nullptr;
	if (!InventorySubsystem)
	{
		return false;
	}

	FAtlasItemHandle GrantedHandle;
	UAtlasInventoryContainerComponent* Backpack = InventorySubsystem->GetFirstContainerOfType(EAtlasContainerType::Backpack);
	if (!InventorySubsystem->AddItem(ItemDefinition, Quantity, Backpack, SourceType, FGameplayTag(), GrantedHandle))
	{
		return false;
	}

	OnCollected(Collector, GrantedHandle);
	Destroy();
	return true;
}
