#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inventory/Runtime/AtlasInventoryTypes.h"
#include "AtlasItemPickupActor.generated.h"

class UAtlasItemDefinition;
class UStaticMeshComponent;

UCLASS(Blueprintable)
class UNICOMBAT_API AAtlasItemPickupActor : public AActor
{
	GENERATED_BODY()

public:
	AAtlasItemPickupActor();

	UFUNCTION(BlueprintCallable, Category = "Inventory|Pickup")
	bool CollectPickup(AActor* Collector);

	UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Pickup")
	void OnCollected(AActor* Collector, const FAtlasItemHandle& GrantedHandle);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Pickup")
	TObjectPtr<UStaticMeshComponent> MeshComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Pickup")
	TObjectPtr<UAtlasItemDefinition> ItemDefinition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Pickup")
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Pickup")
	EAtlasItemSourceType SourceType = EAtlasItemSourceType::WorldPickup;
};
