// Fill out your copyright notice in the Description page of Project Settings.


#include "AtlasGameInstance.h"

void UAtlasGameInstance::Init()
{
	Super::Init();
	PostSubsystemInit();
}

void UAtlasGameInstance::ImportPersistenceSnapshot(const FAtlasGeneralParam& Snapshot)
{
	GeneralParam = Snapshot;
}

void UAtlasGameInstance::PostSubsystemInit_Implementation()
{
}
