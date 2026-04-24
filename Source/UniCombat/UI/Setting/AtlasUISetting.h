#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AtlasUISetting.generated.h"

class UGameHUDWidget;
class UGameMenuWidget;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "UI"))
class UNICOMBAT_API UAtlasUISetting : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "UI")
	TSoftClassPtr<UGameHUDWidget> HUDClass;

	UPROPERTY(EditAnywhere, Config, Category = "UI")
	TSoftClassPtr<UGameMenuWidget> MenuClass;

	UPROPERTY(EditAnywhere, Config, Category = "UI")
	int32 HUDZOrder = 0;

	UPROPERTY(EditAnywhere, Config, Category = "UI")
	int32 MenuZOrder = 50;
};
