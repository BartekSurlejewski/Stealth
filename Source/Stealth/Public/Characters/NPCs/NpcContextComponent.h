#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/AI/Suspicion/NpcSuspicionComponent.h"
#include "NpcContextComponent.generated.h"

class UNpcProfile;
class ANpcAiController;
class AStealthPlayerCharacter;
struct FAIStimulus;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNpcStateChanged, const FGameplayTag&, NewStateTag, const FGameplayTag&, PreviousStateTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBehaviourStateChanged, ENpcBehaviourState, NewBehaviourState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlertLevelChanged, ENpcAlertLevel, NewAlertLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerInSightChanged, bool, IsPlayerInDirectSight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSuspicionChanged, float, NewSuspicion);

/**
 * Context Component acting as an interface and unified data facade
 * between NPC subsystems/components (Suspicion, Focus, Schedule, Profile)
 * and AI systems (StateTrees, Gameplay Messages, external queries).
 */
UCLASS(Blueprintable, ClassGroup = "NPC")
class STEALTH_API UNpcContextComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnNpcStateChanged OnNpcStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnBehaviourStateChanged OnBehaviourStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnAlertLevelChanged OnAlertLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnPlayerInSightChanged OnPlayerInSightChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnSuspicionChanged OnSuspicionChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnNpcFocusChanged OnNpcFocusChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnNpcNoiseHeard OnNoiseHeard;

public:
	UNpcContextComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Perception callbacks (forwarded to SuspicionComponent)
	UFUNCTION()
	virtual void OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus);

	UFUNCTION(BlueprintCallable, Category = "NPC|Perception")
	virtual bool OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);

	/** Handle crime report dispatched from HousingSubsystem */
	UFUNCTION(BlueprintCallable, Category = "NPC|Crime")
	virtual void HandleCrimeReported(const FAiCrimeEventPayload& CrimePayload);

	// State & Data Accessors for StateTree and external systems
	UFUNCTION(BlueprintPure, Category = "NPC|State")
	FGameplayTag GetCurrentStateTag() const { return CurrentStateTag; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	const FGuid& GetNpcGuid() const { return OwnerNpcGuid; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	bool IsPlayerInRestrictedArea() const;

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	virtual bool IsPlayerPerformingIllegalAction(const AStealthPlayerCharacter* Player) const;

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	float GetAwareness() const;

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	ENpcAlertLevel GetAlertLevel() const;

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	ENpcBehaviourState GetBehaviourState() const;

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	bool HasPlayerLineOfSight() const;

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	bool EffectivelySeesPlayer() const;

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	const FVector& GetLastKnownPlayerPos() const;

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	const FVector& GetLastHeardSoundLocation() const;

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	const FNpcFocusTarget& GetCurrentFocus() const;

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	bool IsDistracted() const;

	// Actions & StateTree communication
	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void AddSuspicion(float Amount);

	/** Single source of truth for NPC state */
	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void SetNpcState(const FGameplayTag& NewStateTag);

	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void SetAlertLevel(ENpcAlertLevel NewAlertLevel);

	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void SetBehaviourState(ENpcBehaviourState NewState);

	UFUNCTION(BlueprintCallable, Category = "NPC|Focus")
	bool RequestFocus(const FNpcFocusTarget& NewFocusCandidate);

	UFUNCTION(BlueprintCallable, Category = "NPC|Focus")
	void ClearFocus(ENpcFocusPriority MinimumPriorityToClear = ENpcFocusPriority::None);

	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void SendStateTreeEvent(const FGameplayTag& Tag) const;

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	AStealthPlayerCharacter* GetPlayerCharacter() const;

	// Subcomponent accessors
	UFUNCTION(BlueprintPure, Category = "NPC|Components")
	UNpcSuspicionComponent* GetSuspicionComponent() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Components")
	UNpcFocusComponent* GetFocusComponent() const;

	UFUNCTION(BlueprintPure, Category = "NPC|Profile")
	UNpcProfile* GetProfile() const { return Profile; }

	UFUNCTION(BlueprintCallable, Category = "NPC|Profile")
	void SetProfile(UNpcProfile* InProfile);

protected:
	void BindToSubcomponents();

	UFUNCTION()
	void HandleAlertStateEvaluated(const FGameplayTag& TargetStateTag, ENpcAlertLevel NewAlertLevel);

	UFUNCTION()
	void HandleSuspicionChanged(float NewSuspicion);

	UFUNCTION()
	void HandlePlayerInSightChanged(bool bInSight);

	UFUNCTION()
	void HandleFocusChanged(const FNpcFocusTarget& NewFocus, const FNpcFocusTarget& PreviousFocus);

	UFUNCTION()
	void HandleNoiseHeard(ENpcNoiseType NoiseType, const FVector& NoiseLocation);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Profile")
	TObjectPtr<UNpcProfile> Profile;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|State")
	FGameplayTag CurrentStateTag;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|State")
	FGuid OwnerNpcGuid;

	UPROPERTY()
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;

	UPROPERTY()
	TObjectPtr<ANpcAiController> NpcAiController;

	UPROPERTY()
	mutable TObjectPtr<UNpcSuspicionComponent> SuspicionComponent;

	UPROPERTY()
	mutable TObjectPtr<UNpcFocusComponent> FocusComponent;
};
