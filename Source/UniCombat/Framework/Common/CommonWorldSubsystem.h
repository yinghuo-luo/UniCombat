// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Subsystems/WorldSubsystem.h"
#include "CommonWorldSubsystem.generated.h"

class ACommonWorldSetting;
/**
 * 
 */
UCLASS()
class UNICOMBAT_API UCommonWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void RecoveryQuest(const UWorld* World);
private:
	UPROPERTY()
	const ACommonWorldSetting* CurrentWorldSettings = nullptr;
};
