#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AtlasTransferSettings.generated.h"

class UUserWidget;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Atlas Transfer"))
class UNICOMBAT_API UAtlasTransferSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "Loading")
	TSoftClassPtr<UUserWidget> LoadingWidgetClass;

	UPROPERTY(EditAnywhere, Config, Category = "Loading", meta = (ClampMin = "0"))
	int32 LoadingWidgetZOrder = 10000;

	UPROPERTY(EditAnywhere, Config, Category = "Same Map", meta = (ClampMin = "0.0"))
	float SameMapFadeOutDuration = 0.15f;

	UPROPERTY(EditAnywhere, Config, Category = "Same Map", meta = (ClampMin = "0.0"))
	float SameMapBlackHoldDuration = 0.05f;

	UPROPERTY(EditAnywhere, Config, Category = "Same Map", meta = (ClampMin = "0.0"))
	float SameMapFadeInDuration = 0.15f;
};
