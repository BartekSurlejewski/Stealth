#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "NpcProfile.generated.h"

class UNpcStateProfile;

UCLASS(BlueprintType)
class STEALTH_API UNpcProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	// Suspicion
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Suspicion")
	float SuspicionGainPerSecond_Sight = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Suspicion")
	float SuspicionGainPerSecond_Peripheral = 15.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Suspicion")
	float SuspicionDecayPerSecond = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Suspicion")
	float SuspicionThreshold_Alert = 75.f;

	// Perception
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perception")
	float AlwaysSeePlayerRange = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perception")
	float GainPlayerSightGracePeriod = 1.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Perception")
	float LosePlayerSightGracePeriod = 1.0f;

	// Modifiers
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modifiers")
	float CrouchSuspicionMultiplier = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Modifiers")
	float ShadowSuspicionMultiplier = 0.3f;

	/** State profiles mapped by State GameplayTag (e.g. NPC.State.Combat -> CombatStateProfile) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="States")
	TMap<FGameplayTag, TObjectPtr<UNpcStateProfile>> StateProfiles;

	UFUNCTION(BlueprintPure, Category="NPC|Profile")
	UNpcStateProfile* GetStateProfile(const FGameplayTag& StateTag) const;
};
