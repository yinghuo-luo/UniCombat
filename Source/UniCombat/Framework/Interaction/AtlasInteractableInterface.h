#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AtlasInteractableInterface.generated.h"

class AActor;

UINTERFACE(BlueprintType)
class UNICOMBAT_API UAtlasInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class UNICOMBAT_API IAtlasInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	bool CanInteract(AActor* Interactor) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction")
	void Interact(AActor* Interactor);
};
