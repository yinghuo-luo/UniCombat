// Fill out your copyright notice in the Description page of Project Settings.


#include "GeneralParamAdapter.h"

bool UGeneralParamAdapter::GatherSaveData(const FAtlasSaveExecutionContext& Context, FAtlasProviderRecord& OutRecord)
{
	UAtlasGameInstance* Result = Cast<UAtlasGameInstance>(Context.GameInstance) ;
	if (!Result)
		return false;
	FAtlasGeneralParamPayload Payload;
	Payload.Snapshot = Result->GeneralParam;
	
	return WriteTypedPayloadRecord(Payload, OutRecord);
}

bool UGeneralParamAdapter::ApplyLoadedData(const FAtlasLoadExecutionContext& Context, const FAtlasProviderRecord& Record)
{
	UAtlasGameInstance* Result = Cast<UAtlasGameInstance>(Context.GameInstance) ;
	if (!Result)
		return false;
	FAtlasGeneralParamPayload Payload;
	if (!ReadTypedPayloadRecord(Record, Payload))
		return false;
	Result->ImportPersistenceSnapshot(Payload.Snapshot);
	return true;
}