#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Components/ActorComponent.h"
#include "GuardNpcContextComponent.generated.h"

class UStateTreeComponent;
class UNpcPatrolComponent;
struct FAIStimulus;
class UGuardNpcProfile;

UENUM(BlueprintType)
enum class EGuardBehaviourState : uint8
{
	Patrol, Suspicious, Alerted, Search, Alarm
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBehaviourStateChanged, EGuardBehaviourState, NewBehaviourState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerInSightChanged, bool, IsPlayerInDirectSight, bool, IsPlayerInPeripheralSight);


UCLASS(ClassGroup="NPC", meta=(BlueprintSpawnableComponent))
class STEALTH_API UGuardNpcContextComponent : public UNpcContextComponent
{
	GENERATED_BODY()

	/*Events*/
public:
	UPROPERTY(BlueprintAssignable)
	FOnBehaviourStateChanged OnBehaviourStateChanged;
	UPROPERTY(BlueprintAssignable)
	FOnPlayerInSightChanged OnPlayerInSightChanged;

	/*Properties*/
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Guard")
	TObjectPtr<UGuardNpcProfile> Profile;

	// State (read from State Tree)
	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	EGuardBehaviourState BehaviourState = EGuardBehaviourState::Patrol;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	float SearchTimer = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	int32 GlobalAlarmLevel = 0;

	// Patrol
	UPROPERTY(BlueprintReadWrite, Category="Guard|Patrol")
	int32 CurrentPatrolIndex = 0;

private:
	UPROPERTY()
	TObjectPtr<UNpcPatrolComponent> PatrolComponent;
	UPROPERTY()
	TSoftObjectPtr<APawn> PlayerPawn;

	UPROPERTY(EditDefaultsOnly, Category="Guard|State|Tags")
	FGameplayTag SearchExpiredTag;
	UPROPERTY(EditDefaultsOnly, Category="Guard|State|Tags")
	FGameplayTag AlertThresholdMetTag;
	UPROPERTY(EditDefaultsOnly, Category="Guard|State|Tags")
	FGameplayTag GlobalAlarmTag;

	/*Methods*/
public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION(BlueprintPure)
	AActor* GetCurrentPatrolPoint() const;
	UFUNCTION(BlueprintCallable)
	void IncrementPatrolIndex() const;

	// External systems API
	UFUNCTION(BlueprintCallable)
	void SetBehaviourState(const EGuardBehaviourState& NewBehaviourState)
	{
		if (BehaviourState != NewBehaviourState)
		{
			BehaviourState = NewBehaviourState;
			OnBehaviourStateChanged.Broadcast(BehaviourState);
		}
	}

	UFUNCTION(BlueprintCallable)
	void ForceAlert(FVector AtLocation);

	UFUNCTION(BlueprintCallable)
	void BeginSearch();

	UFUNCTION(BlueprintPure)
	bool IsSearchExpired() const { return SearchTimer <= 0.f; }

	UFUNCTION(BlueprintPure)
	bool IsOnWalkingPatrol() const;

	// Perception callbacks
	void OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus, float ExposureMultiplier);
	void OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);

	UFUNCTION(BlueprintCallable)
	void LookAtPlayer();

private:
	void SendGuardEvent(FGameplayTag Tag) const;
	void OnAlarmChanged(int32 NewLevel, const FVector& SourceLocation);
	float GetSuspicionModifier(AActor* Target) const;
	APawn* GetPlayerPawn();
};
