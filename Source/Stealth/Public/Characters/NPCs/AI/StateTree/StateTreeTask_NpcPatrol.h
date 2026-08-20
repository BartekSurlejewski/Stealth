#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "StateTreeTask_NpcPatrol.generated.h"

class ANpcAiController;
class UNpcScheduleComponent;
class UGuardNpcContextComponent;
class APatrolRoute;

USTRUCT()
struct STEALTH_API FStateTreeTask_NpcPatrolInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ANpcAiController> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UNpcScheduleComponent> ScheduleComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UGuardNpcContextComponent> GuardContext = nullptr;

	/** Location tag for the patrol route. If not set, reads from ScheduleComponent active location */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag LocationTag;

	/** Acceptance radius in units for reaching each waypoint */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 75.0f;

	/** Override wait time at waypoints (-1 to use Route's defined wait time) */
	UPROPERTY(EditAnywhere, Category = "Parameter")
	float WaitTimeOverride = -1.0f;

	UPROPERTY()
	TWeakObjectPtr<APatrolRoute> ActiveRoute = nullptr;

	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentTargetWaypoint = nullptr;

	UPROPERTY()
	float CurrentWaitTimer = 0.0f;

	UPROPERTY()
	bool bIsWaitingAtWaypoint = false;
};

USTRUCT(meta = (DisplayName = "NPC Patrol Route", Category = "NPC"))
struct STEALTH_API FStateTreeTask_NpcPatrol : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeTask_NpcPatrolInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
