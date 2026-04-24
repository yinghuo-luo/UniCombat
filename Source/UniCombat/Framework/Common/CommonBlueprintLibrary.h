// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonBlueprintLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UNICOMBAT_API UCommonBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable,BlueprintPure,Category="Framework|Common")
	static FName GetAutosaveName();
};
