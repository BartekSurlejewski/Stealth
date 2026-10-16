#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "NpcProfile.generated.h"

UCLASS(BlueprintType)
class STEALTH_API UNpcProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	UNpcProfile();

	// Suspicion
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspicion")
	float SuspicionGainPerSecond_Sight = 40.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspicion")
	float SuspicionGainPerSecond_Peripheral = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspicion")
	float SuspicionDecayPerSecond = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspicion")
	float SuspicionThreshold_Suspicious = 25.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspicion")
	float SuspicionThreshold_Alert = 75.0f;

	// Named suspicion values replacing magic numbers
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspicion")
	float TouchSuspicionGain = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Suspicion")
	float DamageSuspicion = 100.0f;

	// Perception
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception")
	float AlwaysSeePlayerRange = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception")
	float LosePlayerSightGracePeriod = 1.0f;

	// Visibility Grade Exposure Thresholds
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception|Visibility")
	float VisibilityExposureThreshold_Shadowed = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception|Visibility")
	float VisibilityExposureThreshold_FullyVisible = 0.6f;

	// Modifiers
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Modifiers")
	float CrouchSuspicionMultiplier = 0.5f;

	// Noise Classification
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception|Noise")
	TMap<FName, ENpcNoiseType> NoiseTagSeverity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception|Noise")
	float NoiseThreshold_Critical = 0.8f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception|Noise")
	float NoiseThreshold_Major = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Perception|Noise")
	float NoiseThreshold_Distraction = 0.2f;

	// Focus Rules
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Focus")
	TMap<ENpcNoiseType, FNpcFocusRule> NoiseFocusRules;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Focus")
	FNpcFocusRule PlayerSightedFocusRule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Focus")
	FNpcFocusRule PlayerTouchedFocusRule;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Focus")
	FNpcFocusRule PlayerDamagedFocusRule;

	UFUNCTION(BlueprintPure, Category = "Perception|Noise")
	ENpcNoiseType ClassifyNoise(const FName& StimulusTag, float Strength) const;
};
