#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Components/ActorComponent.h"
#include "GuardNpcContextComponent.generated.h"

class AStealthPlayerState;
class UStateTreeComponent;
class UNpcPatrolComponent;
struct FAIStimulus;
class UNpcProfile;

UCLASS(ClassGroup="NPC", meta=(BlueprintSpawnableComponent))
class STEALTH_API UGuardNpcContextComponent : public UNpcContextComponent
{
	GENERATED_BODY()

	/*Properties*/
public:
	// State (read from State Tree)
	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	EGuardBehaviourState BehaviourState = EGuardBehaviourState::Patrol;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	int32 GlobalAlarmLevel = 0;

	// Patrol
	UPROPERTY(BlueprintReadWrite, Category="Guard|Patrol")
	int32 CurrentPatrolIndex = 0;

private:
	UPROPERTY()
	TObjectPtr<UNpcPatrolComponent> PatrolComponent;

	/*Methods*/
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintPure)
	AActor* GetCurrentPatrolPoint() const;
	UFUNCTION(BlueprintCallable)
	void IncrementPatrolIndex() const;

	// External systems API
	UFUNCTION(BlueprintCallable)
	void SetBehaviourState(const EGuardBehaviourState& NewBehaviourState);

	UFUNCTION(BlueprintPure)
	bool IsOnWalkingPatrol() const;

	UFUNCTION(BlueprintCallable)
	void LookAtPlayer();

private:
	UFUNCTION()
	void OnPlayerPerformedIllegalAction();
	UFUNCTION()
	void OnIsPlayerInRestrictedAreaChanged(bool bIsInRestrictedArea);
	UFUNCTION()
	void OnAlarmChanged(int32 NewLevel, const FVector& SourceLocation);
};
