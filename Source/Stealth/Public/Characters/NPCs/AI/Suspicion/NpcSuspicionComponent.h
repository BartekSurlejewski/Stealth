#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "NpcSuspicionComponent.generated.h"

class UNpcProfile;
class AStealthPlayerCharacter;
class ANpcAiController;
class UNpcFocusComponent;
struct FBooleanMessage;
struct FAIStimulus;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNpcSuspicionChanged, float, NewSuspicion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNpcAlertLevelChanged, ENpcAlertLevel, NewAlertLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNpcBehaviourStateChanged, EGuardBehaviourState, NewBehaviourState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNpcPlayerInSightChanged, bool, IsPlayerInDirectSight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNpcAlertStateEvaluated, const FGameplayTag&, TargetStateTag, ENpcAlertLevel, NewAlertLevel);

/**
 * Component responsible exclusively for sensory stimulus processing,
 * line-of-sight & exposure evaluation, suspicion accumulation / decay,
 * and alert level state machine transitions.
 */
UCLASS(ClassGroup = "NPC|Suspicion", meta = (BlueprintSpawnableComponent))
class STEALTH_API UNpcSuspicionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcSuspicionComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Suspicion|Events")
	FOnNpcSuspicionChanged OnSuspicionChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Suspicion|Events")
	FOnNpcAlertLevelChanged OnAlertLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Suspicion|Events")
	FOnNpcBehaviourStateChanged OnBehaviourStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Suspicion|Events")
	FOnNpcPlayerInSightChanged OnPlayerInSightChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Suspicion|Events")
	FOnNpcAlertStateEvaluated OnAlertStateEvaluated;

	// Perception callbacks
	UFUNCTION()
	virtual void OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus);

	UFUNCTION()
	virtual void OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	virtual void HandleCrimeReported(const FAiCrimeEventPayload& CrimePayload);

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void AddSuspicion(float Amount);

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void SetAlertLevel(ENpcAlertLevel NewAlertLevel);

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void SetBehaviourState(EGuardBehaviourState NewState);

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	float GetSuspicion() const { return CurrentSuspicion; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	ENpcAlertLevel GetAlertLevel() const { return AlertLevel; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	EGuardBehaviourState GetBehaviourState() const { return CurrentBehaviourState; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	bool HasPlayerLineOfSight() const { return bHasPlayerLineOfSight; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	bool EffectivelySeesPlayer() const { return bEffectivelySeesPlayer; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	const FVector& GetLastKnownPlayerPos() const { return LastKnownPlayerPos; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	const FVector& GetLastHeardSoundLocation() const { return LastHeardSoundLocation; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	bool IsPlayerInRestrictedArea() const { return bIsPlayerInRestrictedArea; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	float GetTimeSinceLastStimulus() const { return TimeSinceLastStimulus; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	AActor* GetLastPerceivedActor() const { return LastPerceivedActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	virtual bool IsPlayerPerformingIllegalAction(const AStealthPlayerCharacter* Player) const;

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	UNpcProfile* GetProfile() const { return Profile; }

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void SetProfile(UNpcProfile* InProfile) { Profile = InProfile; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	AStealthPlayerCharacter* GetPlayerCharacter() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	UNpcFocusComponent* GetFocusComponent() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	ANpcAiController* GetAiController() const;

protected:
	void UpdateSuspicion(float DeltaTime);
	void EvaluateAlertState();
	float CalculatePlayerExposureMultiplier() const;
	bool IsLookingDirectlyAtPlayer(const AStealthPlayerCharacter* Player) const;

private:
	void OnPlayerInRestrictedAreaChanged(FGameplayTag Channel, const FBooleanMessage& Message);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Profile")
	TObjectPtr<UNpcProfile> Profile;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|Suspicion")
	float CurrentSuspicion = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|Suspicion")
	ENpcAlertLevel AlertLevel = ENpcAlertLevel::Unaware;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|Suspicion")
	EGuardBehaviourState CurrentBehaviourState = EGuardBehaviourState::Patrol;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	FVector LastKnownPlayerPos = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	FVector LastHeardSoundLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	bool bHasPlayerLineOfSight = false;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	bool bEffectivelySeesPlayer = false;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	float TimeSinceLastStimulus = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	TWeakObjectPtr<AActor> LastPerceivedActor = nullptr;

	UPROPERTY()
	bool bIsPlayerInRestrictedArea = false;

private:
	FGameplayMessageListenerHandle PlayerInRestrictedAreaListenerHandle;

	UPROPERTY()
	mutable TWeakObjectPtr<ANpcAiController> CachedController;

	UPROPERTY()
	mutable TWeakObjectPtr<UNpcFocusComponent> CachedFocusComponent;
};
