// Fill out your copyright notice in the Description page of Project Settings.


#include "AtlasPlayerController.h"

#include "Engine/LocalPlayer.h"
#include "UI/Core/UIRootSubsystem.h"

void AAtlasPlayerController::BeginPlay()
{
	Super::BeginPlay();

	/*if (!IsLocalController())
	{
		return;
	}

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UUIRootSubsystem* UIRootSubsystem = LocalPlayer->GetSubsystem<UUIRootSubsystem>())
		{
			UIRootSubsystem->EnsureUI();
		}
	}*/
}
