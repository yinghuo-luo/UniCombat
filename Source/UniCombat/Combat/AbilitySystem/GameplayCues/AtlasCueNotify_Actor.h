// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "AtlasCueNotify_Actor.generated.h"

UCLASS()
class UNICOMBAT_API AAtlasCueNotify_Actor : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AAtlasCueNotify_Actor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
