#pragma once

#include "AtlasCombatMode.generated.h"

UENUM(BlueprintType)
enum class EAtlasCombatMode : uint8
{
	Knife UMETA(DisplayName = "Knife"),
	Stick UMETA(DisplayName = "Stick")
};
