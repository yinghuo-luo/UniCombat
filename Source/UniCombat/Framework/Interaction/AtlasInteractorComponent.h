#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AtlasInteractorComponent.generated.h"

class AActor;

UCLASS(ClassGroup = (Interaction), BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class UNICOMBAT_API UAtlasInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAtlasInteractorComponent();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryInteract();

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	AActor* FindBestInteractableActor() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0"))
	float InteractionDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0"))
	float InteractionSweepRadius = 32.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bUseControllerViewPoint = true;

private:
	bool ResolveInteractionViewPoint(FVector& OutLocation, FVector& OutDirection) const;
	AActor* ResolveInteractableActorFromHit(const FHitResult& HitResult, AActor* Interactor) const;
	AActor* FindBestInteractableActorBySweep(const FVector& Start, const FVector& End, AActor* Interactor) const;
	bool CanInteractWithActor(AActor* CandidateActor, AActor* Interactor) const;
};
