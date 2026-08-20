#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "StateTreeTask_PerformRoutine.generated.h"

class ANpcAiController;
class UNpcScheduleComponent;
class UGuardNpcContextComponent;
class APatrolRoute;

USTRUCT()
struct STEALTH_API FStateTreeTask_PerformRoutineInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ANpcAiController> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UNpcScheduleComponent> ScheduleComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UGuardNpcContextComponent> GuardContext = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	TWeakObjectPtr<AActor> TargetActivityPoint = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag ExpectedActivityTag;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 75.0f;

	UPROPERTY()
	TWeakObjectPtr<APatrolRoute> ActivePatrolRoute = nullptr;

	UPROPERTY()
	float WaypointWaitTimer = 0.0f;

	UPROPERTY()
	bool bIsWaitingAtWaypoint = false;

	UPROPERTY()
	bool bHasArrivedAtActivity = false;

	UPROPERTY()
	FGameplayTag CurrentActivityTag;

	UPROPERTY()
	FGameplayTag CurrentLocationTag;
};

USTRUCT(meta = (DisplayName = "NPC Perform Routine", Category = "NPC"))
struct STEALTH_API FStateTreeTask_PerformRoutine : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeTask_PerformRoutineInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
