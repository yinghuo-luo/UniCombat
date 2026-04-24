// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SaveSystem/AtlasSaveTypes.h"
#include "AtlasSaveBlueprintLibrary.generated.h"

class UAtlasSaveSubsystem;
/**
 *
 */
UCLASS()
class UNICOMBAT_API UAtlasSaveBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Save",meta = (WorldContext = "WorldContextObject"))
	static UAtlasSaveSubsystem* GetSaveSubsystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Save",meta = (WorldContext = "WorldContextObject"))
	static bool SavaManual(const UObject* WorldContextObject, const FString& SlotName,
		const FString& DisplayName);
	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContextObject"))
	static bool SaveAutosave(const UObject* WorldContextObject, FName ContextId);
	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContextObject"))
	static bool SaveCheckpoint(const UObject* WorldContextObject, FName CheckpointId);
	
	UFUNCTION(BlueprintCallable, Category = "Save",meta = (WorldContext = "WorldContextObject"))
	static bool LoadSlot(const UObject* WorldContextObject, const FString& SlotName,int32 UserIndex);
	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContextObject"))
	static bool DeleteSlot(const UObject* WorldContextObject, const FString& SlotName, int32 UserIndex);
	
	UFUNCTION(BlueprintPure, Category = "Save", meta = (WorldContext = "WorldContextObject"))
	static TArray<FAtlasSaveSlotDescriptor> EnumerateSlots(const UObject* WorldContextObject);
	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContextObject"))
	static FAtlasRestoreBatchResult ApplyProviderRecordByName(const UObject* WorldContextObject,EAtlasSaveRestoreName Name);
	UFUNCTION(BlueprintCallable, Category = "Save", meta = (WorldContext = "WorldContextObject"))
	static FAtlasRestoreBatchResult ApplyObjectRecordsByName(const UObject* WorldContextObject,EAtlasSaveRestoreName Name);
};
