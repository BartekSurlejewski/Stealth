#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "Characters/NPCs/AI/States/NpcState.h"
#include "NpcSuspicionComponent.generated.h"

class UNpcProfile;
class AStealthPlayerCharacter;
class ANpcAiController;
class UNpcFocusComponent;
class UNpcContextComponent;
struct FBooleanMessage;
struct FAIStimulus;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNpcSuspicionChanged, float, NewSuspicion);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNpcAlertLevelChanged, ENpcAlertLevel, NewAlertLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNpcBehaviourStateChanged, ENpcBehaviourState, NewBehaviourState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNpcPlayerInSightChanged, bool, IsPlayerInDirectSight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNpcAlertStateEvaluated, const FGameplayTag&, TargetStateTag, ENpcAlertLevel, NewAlertLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNpcNoiseHeard, ENpcNoiseType, NoiseType, const FVector&, NoiseLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNpcStateChanged, const FGameplayTag&, NewStateTag, const FGameplayTag&, PreviousStateTag);

/**
 * Component responsible for single-source-of-truth NPC state management,
 * suspicion accumulation / decay, sensory stimulus routing to polymorphic states,
 * and state transition dispatching.
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

	// Events & Delegates
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

	UPROPERTY(BlueprintAssignable, Category = "NPC|Suspicion|Events")
	FOnNpcNoiseHeard OnNoiseHeard;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Suspicion|Events")
	FOnNpcStateChanged OnNpcStateChanged;

	// Perception & Stimulus Handlers
	UFUNCTION()
	virtual void OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus);

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	virtual bool OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	ENpcNoiseType ClassifyNoiseStimulus(const FAIStimulus& Stimulus) const;

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	bool ShouldReactToNoise(ENpcNoiseType NoiseType, const FAIStimulus& Stimulus) const;

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	virtual void HandleCrimeReported(const FAiCrimeEventPayload& CrimePayload);

	// Suspicion & Alert Mutators
	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void AddSuspicion(float Amount);

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void SetSuspicion(float Value);

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void SetAlertLevel(ENpcAlertLevel NewAlertLevel);

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void SetBehaviourState(ENpcBehaviourState NewState);

	// State Management (Single Source of Truth)
	UFUNCTION(BlueprintPure, Category = "NPC|States")
	const FGameplayTag& GetCurrentStateTag() const { return CurrentStateTag; }

	UFUNCTION(BlueprintCallable, Category = "NPC|States")
	bool TransitionToStateByTag(const FGameplayTag& StateTag);

	UFUNCTION(BlueprintCallable, Category = "NPC|States")
	bool TransitionToState(UNpcState* NewState);

	UFUNCTION(BlueprintPure, Category = "NPC|States")
	UNpcState* GetStateByTag(const FGameplayTag& StateTag) const;

	UFUNCTION(BlueprintPure, Category = "NPC|States")
	UNpcState* GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "NPC|States")
	void RegisterState(UNpcState* NewState);

	UFUNCTION(BlueprintCallable, Category = "NPC|States")
	void UnregisterState(UNpcState* StateToRemove);

	// Getters & Facade Accessors
	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	float GetSuspicion() const { return CurrentSuspicion; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	float GetAwareness() const { return CurrentSuspicion / 100.0f; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	ENpcAlertLevel GetAlertLevel() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	ENpcBehaviourState GetBehaviourState() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	bool HasPlayerLineOfSight() const { return bHasPlayerLineOfSight; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	bool EffectivelySeesPlayer() const { return bEffectivelySeesPlayer; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	const FVector& GetLastKnownPlayerPos() const { return LastKnownPlayerPos; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	const FVector& GetLastHeardSoundLocation() const { return LastHeardSoundLocation; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	AActor* GetLastPerceivedActor() const { return LastPerceivedActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	AActor* GetCurrentStimulusSourceActor() const { return CurrentStimulusSourceActor.Get(); }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	bool IsStimulusFromPlayer() const { return bStimulusIsFromPlayer; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	float GetLostPlayerSightDuration() const { return LostPlayerSightDuration; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	float GetSearchDurationTimer() const { return SearchDurationTimer; }

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void ResetSearchDurationTimer() { SearchDurationTimer = 0.0f; }

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void SetStimulusSource(AActor* SourceActor, bool bFromPlayer)
	{
		CurrentStimulusSourceActor = SourceActor;
		bStimulusIsFromPlayer = bFromPlayer;
	}

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void SetLastKnownPlayerPos(const FVector& Pos) { LastKnownPlayerPos = Pos; }

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void SetLastHeardSoundLocation(const FVector& Pos) { LastHeardSoundLocation = Pos; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	virtual bool IsPlayerPerformingIllegalAction() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	UNpcProfile* GetProfile() const { return Profile; }

	UFUNCTION(BlueprintCallable, Category = "NPC|Suspicion")
	void SetProfile(UNpcProfile* InProfile) { Profile = InProfile; }

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	AStealthPlayerCharacter* GetPlayerCharacter() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	UNpcFocusComponent* GetFocusComponent() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	UNpcContextComponent* GetContextComponent() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Suspicion")
	ANpcAiController* GetAiController() const;

	UFUNCTION(BlueprintCallable, Category = "NPC|States")
	void InitializeDefaultStates();

protected:
	void UpdatePerceptionState(float DeltaTime);
	float CalculatePlayerExposureMultiplier() const;
	bool IsLookingDirectlyAtPlayer(const AStealthPlayerCharacter* Player) const;
	FNpcStateTickContext BuildTickContext(float DeltaTime) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Profile")
	TObjectPtr<UNpcProfile> Profile;

	/** Configured states for this NPC. If empty, default states will be instantiated at BeginPlay. */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "NPC|States")
	TArray<TObjectPtr<UNpcState>> States;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|States")
	TMap<FGameplayTag, TObjectPtr<UNpcState>> StateRegistry;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|States")
	FGameplayTag CurrentStateTag;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|States")
	TObjectPtr<UNpcState> CurrentState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|Suspicion")
	float CurrentSuspicion = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	FVector LastKnownPlayerPos = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	FVector LastHeardSoundLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	bool bHasPlayerLineOfSight = false;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	bool bEffectivelySeesPlayer = false;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	float LostPlayerSightDuration = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	float SearchDurationTimer = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	TWeakObjectPtr<AActor> LastPerceivedActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	TWeakObjectPtr<AActor> CurrentStimulusSourceActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|Suspicion")
	bool bStimulusIsFromPlayer = false;

private:
	FGameplayMessageListenerHandle PlayerInRestrictedAreaListenerHandle;

	UPROPERTY()
	mutable TWeakObjectPtr<ANpcAiController> CachedController;

	UPROPERTY()
	mutable TWeakObjectPtr<UNpcFocusComponent> CachedFocusComponent;

	UPROPERTY()
	mutable TWeakObjectPtr<UNpcContextComponent> CachedContextComponent;

	UPROPERTY()
	mutable TWeakObjectPtr<AStealthPlayerCharacter> CachedPlayerCharacter;
};
