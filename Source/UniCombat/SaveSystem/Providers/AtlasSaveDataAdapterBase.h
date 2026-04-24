// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveSystem/Core/AtlasSaveSerialization.h"
#include "SaveSystem/Core/AtlasSaveSubsystem.h"
#include "UObject/Object.h"
#include "AtlasSaveDataAdapterBase.generated.h"

/**
 * 
 */
UCLASS(Abstract,Blueprintable,EditInlineNew,DefaultToInstanced)
class UNICOMBAT_API UAtlasSaveDataAdapterBase : public UObject
{
	GENERATED_BODY()
public:
	virtual void InitializeAdapter(UAtlasSaveSubsystem* InOwnerSubsystem);
	virtual void DeinitializeAdapter();
	
	// 	这是 Unreal 的宏，用来模拟/实现“纯虚函数”的效果。
	// 在标准 C++ 里，通常会写成：
	// virtual FName GetProviderId() const = 0;
	// 	但在 Unreal 里，很多 UObject 类会用 PURE_VIRTUAL，因为它除了表示“必须由子类实现”，还会提供一个兜底函数体，
	// 	避免某些 UObject 场景下纯虚调用带来的问题。
	virtual FName GetProviderId() const PURE_VIRTUAL(UAtlasSaveDataAdapterBase::GetProviderId,return NAME_None;);

	virtual int32 GetProviderVersion() const { return 1; }
	virtual FString GetDebugName() const;
	virtual bool SupportsSaveType(EAtlasSaveType SaveType) const {return true;}
	
	virtual EAtlasSaveRestoreName GetRestoreName() const {return EAtlasSaveRestoreName::None;}
	
	virtual void OnPreSave(const FAtlasSaveExecutionContext& Context);
	//实际收集数据
	virtual bool GatherSaveData(const FAtlasSaveExecutionContext& Context,FAtlasProviderRecord& OutRecord);
	virtual void OnPostSave(const FAtlasSaveExecutionContext& Context,const FAtlasProviderRecord* Record);
	
	virtual void OnPreLoad(const FAtlasLoadExecutionContext& Context, const FAtlasProviderRecord* Record);
	virtual bool ApplyLoadedData(const FAtlasLoadExecutionContext& Context, const FAtlasProviderRecord& Record);
	virtual void OnPostLoad(const FAtlasLoadExecutionContext& Context, const FAtlasProviderRecord* Record);
	
protected:
	template <typename TStructType>
	bool WriteTypedPayloadRecord(const TStructType& Payload,FAtlasProviderRecord& OutRecord) const
	{
		const int32 ProviderVersion = FMath::Max(GetProviderVersion(), 1);

		OutRecord.ProviderId = GetProviderId();
		OutRecord.ProviderVersion = ProviderVersion;
		OutRecord.LastModified = FDateTime::UtcNow();
		OutRecord.DebugName = GetDebugName();
		OutRecord.RestoreName = GetRestoreName();
		OutRecord.Payload.PayloadType = TStructType::StaticStruct()->GetFName();
		OutRecord.Payload.SchemaVersion = ProviderVersion;
		return FAtlasSaveSerialization::SerializeUStruct(Payload, OutRecord.Payload.Bytes);
	}
	
	template <typename TStructType>
	bool ReadTypedPayloadRecord(const FAtlasProviderRecord& Record, TStructType& OutPayload) const
	{
		const UScriptStruct* ExpectedStruct = TStructType::StaticStruct();
		if (!ExpectedStruct)
		{
			return false;
		}

		if (Record.Payload.PayloadType != ExpectedStruct->GetFName())
		{
			return false;
		}

		if (Record.Payload.Bytes.IsEmpty())
		{
			return false;
		}

		return FAtlasSaveSerialization::DeserializeUStruct(Record.Payload.Bytes, OutPayload);
	}
	
protected:
	UPROPERTY(Transient)
	TObjectPtr<UAtlasSaveSubsystem> OwnerSubsystem = nullptr;
};
