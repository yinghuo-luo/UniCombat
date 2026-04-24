// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "CommonWorldSetting.generated.h"

/**
 * 
 */
UCLASS()
class UNICOMBAT_API ACommonWorldSetting : public AWorldSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category=WorldSetting)
	bool bCreateMainQuest = false;
};
