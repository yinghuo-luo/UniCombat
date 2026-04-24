#pragma once

#include "CoreMinimal.h"
#include "Framework/Transfer/AtlasTransferTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AtlasTransferBlueprintLibrary.generated.h"

class UAtlasTransferSubsystem;

UCLASS()
class UNICOMBAT_API UAtlasTransferBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Transfer", meta = (WorldContext = "WorldContextObject"))
	static UAtlasTransferSubsystem* GetTransferSubsystem(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Transfer", meta = (WorldContext = "WorldContextObject"))
	static bool StartTransfer(const UObject* WorldContextObject, const FAtlasTransferActionRequest& Request);
};
