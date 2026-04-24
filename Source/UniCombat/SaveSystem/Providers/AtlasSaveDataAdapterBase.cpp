// Fill out your copyright notice in the Description page of Project Settings.


#include "AtlasSaveDataAdapterBase.h"

void UAtlasSaveDataAdapterBase::InitializeAdapter(UAtlasSaveSubsystem* InOwnerSubsystem)
{
	OwnerSubsystem = InOwnerSubsystem;
}

void UAtlasSaveDataAdapterBase::DeinitializeAdapter()
{
	OwnerSubsystem = nullptr;
}

FString UAtlasSaveDataAdapterBase::GetDebugName() const
{
	return GetClass()->GetName();
}

void UAtlasSaveDataAdapterBase::OnPreSave(const FAtlasSaveExecutionContext& Context)
{
}

bool UAtlasSaveDataAdapterBase::GatherSaveData(const FAtlasSaveExecutionContext& Context,
	FAtlasProviderRecord& OutRecord)
{
	return false;
}

void UAtlasSaveDataAdapterBase::OnPostSave(const FAtlasSaveExecutionContext& Context,
	const FAtlasProviderRecord* Record)
{
}

void UAtlasSaveDataAdapterBase::OnPreLoad(const FAtlasLoadExecutionContext& Context, const FAtlasProviderRecord* Record)
{
}

bool UAtlasSaveDataAdapterBase::ApplyLoadedData(const FAtlasLoadExecutionContext& Context,
	const FAtlasProviderRecord& Record)
{
	return false;
}

void UAtlasSaveDataAdapterBase::OnPostLoad(const FAtlasLoadExecutionContext& Context,
	const FAtlasProviderRecord* Record)
{
}
