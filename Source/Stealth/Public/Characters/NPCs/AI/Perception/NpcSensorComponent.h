#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Perception/AIPerceptionTypes.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "NpcSensorComponent.generated.h"

class ANpcAiController;
class UNpcProfile;
class UPlayerExposureSubsystem;
class ULegalitySubsystem;

/**
 * Sensor component responsible exclusively for receiving stimuli from Unreal AIPerception
 * and turning them into raw, state-free facts.
 * Does NOT own suspicion math, focus arbitration, or state transitions.
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class STEALTH_API UNpcSensorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcSensorComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Stimulus Handlers called from ANpcAiController
	void OnSightStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	void OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	void OnPredictionStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	void OnTouchStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	void OnDamageStimulus(AActor* Actor, const FAIStimulus& Stimulus);

	// Getters for computed state-free facts
	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	bool HasLineOfSight() const { return bHasLineOfSight; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	bool IsLineOfSightValid() const { return bLineOfSightValid; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	bool IsLineOfSightBlocked() const { return bLineOfSightBlocked; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	float GetPlayerDistance() const { return PlayerDistance; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	bool IsLookingDirectlyAtPlayer() const { return bIsLookingDirectlyAtPlayer; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	FVector GetLastKnownPlayerPos() const { return LastKnownPlayerPos; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	FVector GetLastHeardSoundLocation() const { return LastHeardSoundLocation; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	FGameplayTag GetLastStimulusTag() const { return LastStimulusTag; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	AActor* GetLastPerceivedActor() const { return LastPerceivedActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	float GetPlayerExposureMultiplier() const { return PlayerExposureMultiplier; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	float GetHearingStimulusStrength() const { return HearingStimulusStrength; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	float GetLastValidObservationTime() const { return LastValidObservationTime; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	float GetLastHeardSoundTime() const { return LastHeardSoundTime; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	bool IsInDetectionRange() const { return bInDetectionRange; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	bool IsFullyVisibleByDistance() const { return bIsFullyVisibleByDistance; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	bool IsPlayerPerformingIllegalAction() const { return bIsPlayerPerformingIllegalAction; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	float GetTimeSinceLastStimulus() const { return TimeSinceLastStimulus; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	bool WasTouchSensedThisTick() const { return bTouchSensedThisTick; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	bool WasDamageSensedThisTick() const { return bDamageSensedThisTick; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Sensor")
	UNpcProfile* GetNpcProfile() const;

	ENpcNoiseType ClassifyNoiseStimulus(const FAIStimulus& Stimulus) const;
	float GetConfiguredSightRadius() const;

protected:
	void UpdateSensoryState(float DeltaTime);
	void UpdateLineOfSight(float DeltaTime);
	void UpdatePlayerLegality();

	ANpcAiController* GetAiController() const;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Profile")
	TObjectPtr<UNpcProfile> NpcProfile;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	bool bHasLineOfSight = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	bool bLineOfSightValid = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	bool bLineOfSightBlocked = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	float PlayerDistance = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	bool bIsLookingDirectlyAtPlayer = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	FVector LastKnownPlayerPos = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	FVector LastHeardSoundLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	FGameplayTag LastStimulusTag = FGameplayTag::EmptyTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	TWeakObjectPtr<AActor> LastPerceivedActor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	float PlayerExposureMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	float HearingStimulusStrength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	float LastValidObservationTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	float LastHeardSoundTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	bool bInDetectionRange = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	bool bIsFullyVisibleByDistance = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	bool bIsPlayerPerformingIllegalAction = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	float TimeSinceLastStimulus = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	bool bTouchSensedThisTick = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Sensor")
	bool bDamageSensedThisTick = false;

private:
	mutable TWeakObjectPtr<ANpcAiController> CachedController;
	TWeakObjectPtr<UPlayerExposureSubsystem> CachedExposureSubsystem;
	TWeakObjectPtr<ULegalitySubsystem> CachedLegalitySubsystem;

	bool bWasPreviouslySensedBySight = false;
};
