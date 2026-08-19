#pragma once

#include "CoreMinimal.h"
#include "Characters/Player/StealthPlayerState.h"
#include "Components/ActorComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "NpcContextComponent.generated.h"

class UPlayerExposureSubsystem;
struct FBooleanMessage;
struct FAIStimulus;
class UNpcProfile;
class ANpcAiController;

UENUM(BlueprintType)
enum class EGuardBehaviourState : uint8
{
	Patrol,
	Suspicious,
	Alerted,
	Search,
	Alarm
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBehaviourStateChanged, EGuardBehaviourState, NewBehaviourState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAlertLevelChanged, ENpcAlertLevel, NewAlertLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerInSightChanged, bool, IsPlayerInDirectSight);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSuspicionChanged, float, NewSuspicion);

UCLASS(Abstract, Blueprintable, ClassGroup = "NPC")
class STEALTH_API UNpcContextComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnBehaviourStateChanged OnBehaviourStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnAlertLevelChanged OnAlertLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnPlayerInSightChanged OnPlayerInSightChanged;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnSuspicionChanged OnSuspicionChanged;

public:
	UNpcContextComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Perception callbacks
	UFUNCTION()
	virtual void OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus);

	UFUNCTION()
	virtual void OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);

	/** Handle crime report dispatched from HousingSubsystem */
	UFUNCTION(BlueprintCallable, Category = "NPC|Crime")
	virtual void HandleCrimeReported(const FAiCrimeEventPayload& CrimePayload);

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	bool IsPlayerInRestrictedArea() const { return bIsPlayerInRestrictedArea; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	float GetAwareness() const { return CurrentSuspicion; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	ENpcAlertLevel GetAlertLevel() const { return AlertLevel; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	EGuardBehaviourState GetBehaviourState() const { return CurrentBehaviourState; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	bool HasPlayerLineOfSight() const { return bHasPlayerLineOfSight; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	bool EffectivelySeesPlayer() const { return bEffectivelySeesPlayer; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	const FVector& GetLastKnownPlayerPos() const { return LastKnownPlayerPos; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	const FVector& GetLastHeardSoundLocation() const { return LastHeardSoundLocation; }

	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void AddSuspicion(float Amount);

	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void SetAlertLevel(ENpcAlertLevel NewAlertLevel);

	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void SetBehaviourState(EGuardBehaviourState NewState);

	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void SendStateTreeEvent(const FGameplayTag& Tag) const;

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	AStealthPlayerCharacter* GetPlayerCharacter() const;

protected:
	void UpdateSuspicion(float DeltaTime);
	void EvaluateAlertState();
	float CalculatePlayerExposureMultiplier() const;
	bool IsLookingDirectlyAtPlayer(const AStealthPlayerCharacter* Player) const;

private:
	void OnPlayerInRestrictedAreaChanged(FGameplayTag Channel, const FBooleanMessage& Message);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	TObjectPtr<UNpcProfile> Profile;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|State")
	float CurrentSuspicion = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|State")
	ENpcAlertLevel AlertLevel = ENpcAlertLevel::Unaware;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|State")
	EGuardBehaviourState CurrentBehaviourState = EGuardBehaviourState::Patrol;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
	FVector LastKnownPlayerPos = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
	FVector LastHeardSoundLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
	bool bHasPlayerLineOfSight = false;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
	bool bEffectivelySeesPlayer = false;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
	float TimeSinceLastStimulus = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "NPC|State")
	TWeakObjectPtr<AActor> LastPerceivedActor = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "NPC|State|Tags")
	FGameplayTag SuspiciousActivityTag;

	UPROPERTY(EditDefaultsOnly, Category = "NPC|State|Tags")
	FGameplayTag AlertedActivityTag;

	UPROPERTY(EditDefaultsOnly, Category = "NPC|State|Tags")
	FGameplayTag AlarmActivityTag;

	UPROPERTY(EditDefaultsOnly, Category = "NPC|State|Tags")
	FGameplayTag InvestigateActivityTag;

	UPROPERTY()
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;

	UPROPERTY()
	TObjectPtr<ANpcAiController> NpcAiController;

private:
	UPROPERTY()
	bool bIsPlayerInRestrictedArea = false;

	FGameplayMessageListenerHandle PlayerInRestrictedAreaListenerHandle;
};
