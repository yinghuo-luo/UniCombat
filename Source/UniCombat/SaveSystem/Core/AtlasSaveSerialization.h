// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


struct UNICOMBAT_API FAtlasSaveSerialization
{
	static bool SerializeUStructBytes(const UScriptStruct* StructType, const void* StructData, TArray<uint8>& OutBytes);
	//反序列化结构体字节
	static bool DeserializeUStructBytes(const UScriptStruct* StructType, const TArray<uint8>& InBytes, void* OutStructData);

	template <typename TStructType>
	static bool SerializeUStruct(const TStructType& InStruct, TArray<uint8>& OutBytes)
	{
		return SerializeUStructBytes(TStructType::StaticStruct(), &InStruct, OutBytes);
	}

	template <typename TStructType>
	static bool DeserializeUStruct(const TArray<uint8>& InBytes, TStructType& OutStruct)
	{
		return DeserializeUStructBytes(TStructType::StaticStruct(), InBytes, &OutStruct);
	}
};
