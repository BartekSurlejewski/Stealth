#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "NpcState.generated.h"

class UNpcContextComponent;
class UNpcSuspicionComponent;
class UNpcState;
class UAnimMontage;

/**
 * Configuration profile for an NPC state.
 * Allows defining movement speed, animations, focus settings, and suspicion modifiers.
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class STEALTH_API UNpcStateProfile : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxWalkSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus")
	ENpcFocusPriority DefaultFocusPriority = ENpcFocusPriority::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FGameplayTag StanceTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> EnterStateMontage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspicion")
	float SuspicionGainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspicion")
	float SuspicionDecayMultiplier = 1.0f;

	/** Apply profile settings (e.g. movement speed) to the NPC */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NPC|StateProfile")
	void ApplyToNpc(UNpcContextComponent* Context);
	virtual void ApplyToNpc_Implementation(UNpcContextComponent* Context);
};

/**
 * Base class for all NPC states.
 * Encapsulates alert level, behavior state, priority, state profile, and polymorphic
 * suspicion computation, transition evaluation, and stimulus handling.
 * Extensible by adding a single UNpcState subclass without modifying existing states or components.
 */
UCLASS(BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class STEALTH_API UNpcState : public UObject
{
	GENERATED_BODY()

public:
	UNpcState();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	FGameplayTag StateTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ENpcAlertLevel AlertLevel = ENpcAlertLevel::Unaware;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	ENpcBehaviourState BehaviourState = ENpcBehaviourState::Routine;

	/** Priority of this state. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	int32 Priority = 0;

	/** Profile containing speed, animations, behavior and focus parameters for this state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, Category = "State|Profile")
	TObjectPtr<UNpcStateProfile> StateProfile;

public:
	UFUNCTION(BlueprintPure, Category = "NPC|State")
	const FGameplayTag& GetStateTag() const { return StateTag; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	ENpcAlertLevel GetAlertLevel() const { return AlertLevel; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	ENpcBehaviourState GetBehaviourState() const { return BehaviourState; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	int32 GetPriority() const { return Priority; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	UNpcStateProfile* GetStateProfile() const { return StateProfile; }

	/** Called when entering this state */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NPC|State")
	void OnEnterState(UNpcContextComponent* Context, UNpcState* PreviousState);
	virtual void OnEnterState_Implementation(UNpcContextComponent* Context, UNpcState* PreviousState);

	/** Called when exiting this state */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NPC|State")
	void OnExitState(UNpcContextComponent* Context, UNpcState* NextState);
	virtual void OnExitState_Implementation(UNpcContextComponent* Context, UNpcState* NextState);

	/** Called on tick while this state is active */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NPC|State")
	void OnTickState(UNpcContextComponent* Context, float DeltaTime);
	virtual void OnTickState_Implementation(UNpcContextComponent* Context, float DeltaTime);

	/** Calculates the suspicion delta to add/subtract this tick while this state is active */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NPC|State")
	float GetSuspicionDelta(const FNpcStateTickContext& Ctx) const;
	virtual float GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const;

	/** Evaluates whether this state wants to transition to another state based on tick context. Returns EmptyTag if staying. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NPC|State")
	FGameplayTag EvaluateTransition(const FNpcStateTickContext& Ctx) const;
	virtual FGameplayTag EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const;

	/** Called when seeing an actor while in this state. Returns desired transition tag or EmptyTag. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NPC|State")
	FGameplayTag HandleSightStimulus(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus);
	virtual FGameplayTag HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus);

	/** Called when hearing a stimulus while in this state. Returns desired transition tag or EmptyTag. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NPC|State")
	FGameplayTag HandleHearingStimulus(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType);
	virtual FGameplayTag HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType);

	/** Called when a crime report is received while in this state. Returns desired transition tag or EmptyTag. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NPC|State")
	FGameplayTag HandleCrimeReported(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload);
	virtual FGameplayTag HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload);

	/** Determines whether transition to a new candidate state is allowed */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "NPC|State")
	bool CanTransitionTo(UNpcContextComponent* Context, const UNpcState* CandidateState) const;
	virtual bool CanTransitionTo_Implementation(UNpcContextComponent* Context, const UNpcState* CandidateState) const;

	/** Applies the state profile to the NPC character and components */
	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	virtual void ApplyStateProfile(UNpcContextComponent* Context);
};

/** Default Unaware State (Routine / Patrol) */
UCLASS(BlueprintType, Blueprintable)
class STEALTH_API UNpcState_Unaware : public UNpcState
{
	GENERATED_BODY()

public:
	UNpcState_Unaware();
	virtual float GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const override;
	virtual FGameplayTag EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const override;
	virtual FGameplayTag HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus) override;
	virtual FGameplayTag HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType) override;
	virtual FGameplayTag HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload) override;
};

/** Default Suspicious State */
UCLASS(BlueprintType, Blueprintable)
class STEALTH_API UNpcState_Suspicious : public UNpcState
{
	GENERATED_BODY()

public:
	UNpcState_Suspicious();
	virtual float GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const override;
	virtual FGameplayTag EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const override;
	virtual FGameplayTag HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus) override;
	virtual FGameplayTag HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType) override;
	virtual FGameplayTag HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload) override;
};

/** Default Alerted / Investigating State */
UCLASS(BlueprintType, Blueprintable)
class STEALTH_API UNpcState_Alerted : public UNpcState
{
	GENERATED_BODY()

public:
	UNpcState_Alerted();
	virtual float GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const override;
	virtual FGameplayTag EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const override;
	virtual FGameplayTag HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus) override;
	virtual FGameplayTag HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType) override;
	virtual FGameplayTag HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload) override;
};

/** Default Search State */
UCLASS(BlueprintType, Blueprintable)
class STEALTH_API UNpcState_Search : public UNpcState
{
	GENERATED_BODY()

public:
	UNpcState_Search();
	virtual float GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const override;
	virtual FGameplayTag EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const override;
	virtual FGameplayTag HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus) override;
	virtual FGameplayTag HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType) override;
	virtual FGameplayTag HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload) override;
};

/** Default Combat / Hostile State */
UCLASS(BlueprintType, Blueprintable)
class STEALTH_API UNpcState_Combat : public UNpcState
{
	GENERATED_BODY()

public:
	UNpcState_Combat();
	virtual float GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const override;
	virtual FGameplayTag EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const override;
	virtual FGameplayTag HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus) override;
	virtual FGameplayTag HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType) override;
	virtual FGameplayTag HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload) override;
};

/** Extensible Demo State: Fleeing (demonstrates zero-edit additions to the open/closed state system) */
UCLASS(BlueprintType, Blueprintable)
class STEALTH_API UNpcState_Fleeing : public UNpcState
{
	GENERATED_BODY()

public:
	UNpcState_Fleeing();
	virtual float GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const override;
	virtual FGameplayTag EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const override;
	virtual FGameplayTag HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus) override;
	virtual FGameplayTag HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType) override;
	virtual FGameplayTag HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload) override;
};
