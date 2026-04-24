// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "QuestSystemSettings.generated.h"

/**
 *
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="QuestSystem"))
class UNICOMBAT_API UQuestSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Config, Category="Quest|Data")
	TSoftObjectPtr<UDataTable> QuestRegistryTableAsset;
};
