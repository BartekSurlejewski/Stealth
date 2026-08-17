#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
enum class EUILayer : uint8
{
	None UMETA(DisplayName = "None"),
	HUD UMETA(DisplayName = "HUD"), // always visible during gameplay
	Overlay UMETA(DisplayName = "Overlay"), // notifications, interaction prompts
	Menu UMETA(DisplayName = "Menu"), // pause, inventory, settings - stackable
	Popup UMETA(DisplayName = "Popup"), // always on top, modal
	WorldSpace UMETA(DisplayName = "WorldSpace"),
	Loading UMETA(DisplayName = "Loading") // absolute top layer
};
