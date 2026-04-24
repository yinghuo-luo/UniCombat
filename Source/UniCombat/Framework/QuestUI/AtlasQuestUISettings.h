#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "AtlasQuestUISettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Quest UI"))
class UNICOMBAT_API UAtlasQuestUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Config, Category = "QuestUI")
	bool bAutoTrackFirstActiveQuestWhenUnset = true;

	UPROPERTY(EditAnywhere, Config, Category = "QuestUI")
	bool bAutoTrackNextActiveQuestWhenTrackedQuestStops = true;
};
