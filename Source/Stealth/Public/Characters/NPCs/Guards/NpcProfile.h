#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NpcProfile.generated.h"

UCLASS()
class STEALTH_API UNpcProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	// Suspicion
	UPROPERTY(EditDefaultsOnly, Category="Suspicion")
	float SuspicionGainPerSecond_Sight = 40.f;

	UPROPERTY(EditDefaultsOnly, Category="Suspicion")
	float SuspicionGainPerSecond_Peripheral = 15.f;

	UPROPERTY(EditDefaultsOnly, Category="Suspicion")
	float SuspicionDecayPerSecond = 8.f;

	UPROPERTY(EditDefaultsOnly, Category="Suspicion")
	float SuspicionThreshold_Alert = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Perception")
	float LosePlayerSightGracePeriod = 1.0f;

	// Modifiers
	UPROPERTY(EditDefaultsOnly, Category="Modifiers")
	float CrouchSuspicionMultiplier = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Modifiers")
	float ShadowSuspicionMultiplier = 0.3f;
};
