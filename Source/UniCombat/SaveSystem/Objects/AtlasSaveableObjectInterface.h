// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SaveSystem/AtlasSaveTypes.h"
#include "UObject/Interface.h"
#include "AtlasSaveableObjectInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UAtlasSaveableObjectInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class UNICOMBAT_API IAtlasSaveableObjectInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	FGuid GetSaveObjectId() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	bool GetSaveIdentity(FAtlasSaveObjectIdentity& OutIdentity) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	bool CaptureSaveData(FAtlasSavePayload& OutPayload) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	void RestoreSaveData(const FAtlasSavePayload& Payload);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	bool ShouldSave() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	bool IsSaveDirty() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	EAtlasSaveRestoreName GetRestoreName() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Save")
	bool ShouldCaptureTransform() const;
};
