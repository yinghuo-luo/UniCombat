// Fill out your copyright notice in the Description page of Project Settings.


#include "AtlasSaveSerialization.h"

#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

namespace
{
	class FAtlasSaveGameArchive : public FObjectAndNameAsStringProxyArchive
	{
	public:
		explicit FAtlasSaveGameArchive(FArchive& InInnerArchive)
			: FObjectAndNameAsStringProxyArchive(InInnerArchive, true)
		{
			ArIsSaveGame = true;
			ArNoDelta = true;
		}
	};
}

bool FAtlasSaveSerialization::SerializeUStructBytes(const UScriptStruct* StructType,
	const void* StructData, TArray<uint8>& OutBytes)
{
	if (!StructType || !StructData)
	{
		return false;
	}

	OutBytes.Reset();
	FMemoryWriter MemoryWriter(OutBytes, true);
	FAtlasSaveGameArchive Archive(MemoryWriter);
	const_cast<UScriptStruct*>(StructType)->SerializeItem(Archive, const_cast<void*>(StructData), nullptr);
	return !Archive.IsError();
}

bool FAtlasSaveSerialization::DeserializeUStructBytes(const UScriptStruct* StructType, const TArray<uint8>& InBytes, void* OutStructData)
{
	if (!StructType || !OutStructData || InBytes.IsEmpty())
	{
		return false;
	}

	FMemoryReader MemoryReader(InBytes, true);
	FAtlasSaveGameArchive Archive(MemoryReader);
	const_cast<UScriptStruct*>(StructType)->SerializeItem(Archive, OutStructData, nullptr);
	return !Archive.IsError();
}