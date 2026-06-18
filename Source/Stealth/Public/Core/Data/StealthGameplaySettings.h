#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "StealthGameplaySettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta=(DisplayName="Stealth Gameplay Settings"))
class STEALTH_API UStealthGameplaySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category="NPCs|Perception")
	float AlwaysSeePlayerRange = 100.0f;
	UPROPERTY(Config, EditAnywhere, Category="NPCs|Perception")
	float PlayerSeeTimeThreshold = 1.0f;

	UFUNCTION(BlueprintCallable)
	static const UStealthGameplaySettings* GetStealthGameplaySettings();
};
