// Fill out your copyright notice in the Description page of Project Settings.


#include "AtlasNPCBase.h"


// Sets default values
AAtlasNPCBase::AAtlasNPCBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AAtlasNPCBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAtlasNPCBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AAtlasNPCBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

bool AAtlasNPCBase::CanInteract_Implementation(AActor* Interactor) const
{
	return bInteractionEnabled && BP_CanInteract(Interactor);
}

void AAtlasNPCBase::Interact_Implementation(AActor* Interactor)
{
	if (!IAtlasInteractableInterface::Execute_CanInteract(this, Interactor))
	{
		return;
	}

	BP_OnInteract(Interactor);
}

bool AAtlasNPCBase::BP_CanInteract_Implementation(AActor* Interactor) const
{
	return Interactor != nullptr;
}

