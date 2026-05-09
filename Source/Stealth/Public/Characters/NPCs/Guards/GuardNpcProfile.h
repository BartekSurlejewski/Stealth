#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GuardNpcProfile.generated.h"

UCLASS()
class STEALTH_API UGuardNpcProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	// Perception
	UPROPERTY(EditDefaultsOnly, Category="Perception")
	float SightRadius = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category="Perception")
	float LoseSightRadius = 1800.f;

	UPROPERTY(EditDefaultsOnly, Category="Perception")
	float PeripheralVisionAngleDegrees = 60.f;

	UPROPERTY(EditDefaultsOnly, Category="Perception")
	float HearingRadius = 1000.f;

	// Suspicion
	UPROPERTY(EditDefaultsOnly, Category="Suspicion")
	float SuspicionGainPerSecond_Sight = 40.f;

	UPROPERTY(EditDefaultsOnly, Category="Suspicion")
	float SuspicionGainPerSecond_Peripheral = 15.f;

	UPROPERTY(EditDefaultsOnly, Category="Suspicion")
	float SuspicionDecayPerSecond = 8.f;

	UPROPERTY(EditDefaultsOnly, Category="Suspicion")
	float SuspicionThreshold_Alert = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Suspicion")
	float SearchDuration = 20.f;

	// Modifiers
	UPROPERTY(EditDefaultsOnly, Category="Modifiers")
	float CrouchMultiplier = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Modifiers")
	float ShadowMultiplier = 0.3f;
};
