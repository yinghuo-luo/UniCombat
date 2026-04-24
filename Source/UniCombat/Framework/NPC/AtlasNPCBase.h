// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/AtlasCharacterBase.h"
#include "Framework/Interaction/AtlasInteractableInterface.h"
#include "AtlasNPCBase.generated.h"

UCLASS()
class UNICOMBAT_API AAtlasNPCBase : public AAtlasCharacterBase, public IAtlasInteractableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAtlasNPCBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual bool CanInteract_Implementation(AActor* Interactor) const override;
	virtual void Interact_Implementation(AActor* Interactor) override;

protected:
	UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
	bool BP_CanInteract(AActor* Interactor) const;
	virtual bool BP_CanInteract_Implementation(AActor* Interactor) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void BP_OnInteract(AActor* Interactor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool bInteractionEnabled = true;
};
