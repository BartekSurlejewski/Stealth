#include "Characters/NPCs/AI/StateTree/StateTreeTask_NpcPatrol.h"
#include "Characters/NPCs/AI/Patrol/PatrolRoute.h"
#include "Characters/NPCs/AI/Patrol/PatrolSubsystem.h"
#include "Characters/NPCs/AI/Schedule/NpcScheduleComponent.h"
#include "Characters/NPCs/Guards/GuardNpcContextComponent.h"
#include "Characters/NPCs/NpcAiController.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"

EStateTreeRunStatus FStateTreeTask_NpcPatrol::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.Controller)
	{
		InstanceData.Controller = Cast<ANpcAiController>(Context.GetOwner());
		if (!InstanceData.Controller)
		{
			if (const APawn* Pawn = Cast<APawn>(Context.GetOwner()))
			{
				InstanceData.Controller = Cast<ANpcAiController>(Pawn->GetController());
			}
		}
	}

	if (!InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	APawn* Pawn = InstanceData.Controller->GetPawn();

	if (!InstanceData.ScheduleComponent)
	{
		InstanceData.ScheduleComponent = InstanceData.Controller->FindComponentByClass<UNpcScheduleComponent>();
		if (!InstanceData.ScheduleComponent && Pawn)
		{
			InstanceData.ScheduleComponent = Pawn->FindComponentByClass<UNpcScheduleComponent>();
		}
	}

	if (!InstanceData.GuardContext)
	{
		InstanceData.GuardContext = InstanceData.Controller->FindComponentByClass<UGuardNpcContextComponent>();
		if (!InstanceData.GuardContext && Pawn)
		{
			InstanceData.GuardContext = Pawn->FindComponentByClass<UGuardNpcContextComponent>();
		}
	}

	FGameplayTag PatrolTag = InstanceData.LocationTag;
	if (!PatrolTag.IsValid() && InstanceData.ScheduleComponent)
	{
		PatrolTag = InstanceData.ScheduleComponent->GetActiveLocationTag();
	}

	if (InstanceData.GuardContext)
	{
		InstanceData.ActiveRoute = InstanceData.GuardContext->AssignPatrolRouteForLocation(PatrolTag);
	}
	else if (UPatrolSubsystem* Subsystem = UPatrolSubsystem::Get(Context.GetOwner()))
	{
		const FVector Pos = Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector;
		InstanceData.ActiveRoute = Subsystem->GetPatrolRouteByLocationTag(PatrolTag, Pos);
	}

	APatrolRoute* Route = InstanceData.ActiveRoute.Get();
	if (!Route || !Route->HasValidWaypoints())
	{
		return EStateTreeRunStatus::Running;
	}

	if (InstanceData.GuardContext)
	{
		InstanceData.CurrentTargetWaypoint = InstanceData.GuardContext->GetCurrentPatrolPoint();
	}
	else
	{
		InstanceData.CurrentTargetWaypoint = Route->GetWaypoint(0);
	}

	InstanceData.bIsWaitingAtWaypoint = false;
	InstanceData.CurrentWaitTimer = 0.0f;

	if (AActor* Target = InstanceData.CurrentTargetWaypoint.Get())
	{
		InstanceData.Controller->MoveToActor(Target, InstanceData.AcceptanceRadius);
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeTask_NpcPatrol::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	APatrolRoute* Route = InstanceData.ActiveRoute.Get();
	if (!Route || !Route->HasValidWaypoints())
	{
		return EStateTreeRunStatus::Running;
	}

	APawn* Pawn = InstanceData.Controller->GetPawn();
	if (!Pawn)
	{
		return EStateTreeRunStatus::Running;
	}

	if (InstanceData.bIsWaitingAtWaypoint)
	{
		InstanceData.CurrentWaitTimer -= DeltaTime;
		if (InstanceData.CurrentWaitTimer <= 0.0f)
		{
			InstanceData.bIsWaitingAtWaypoint = false;

			if (InstanceData.GuardContext)
			{
				InstanceData.GuardContext->IncrementPatrolIndex();
				InstanceData.CurrentTargetWaypoint = InstanceData.GuardContext->GetCurrentPatrolPoint();
			}
			else
			{
				bool DummyForward = true;
				const int32 NextIdx = Route->GetNextWaypointIndex(0, DummyForward);
				InstanceData.CurrentTargetWaypoint = Route->GetWaypoint(NextIdx);
			}

			if (AActor* Target = InstanceData.CurrentTargetWaypoint.Get())
			{
				InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
				InstanceData.Controller->MoveToActor(Target, InstanceData.AcceptanceRadius);
			}
		}
		return EStateTreeRunStatus::Running;
	}

	AActor* Target = InstanceData.CurrentTargetWaypoint.Get();
	if (!Target)
	{
		if (InstanceData.GuardContext)
		{
			InstanceData.CurrentTargetWaypoint = InstanceData.GuardContext->GetCurrentPatrolPoint();
			Target = InstanceData.CurrentTargetWaypoint.Get();
		}
		if (!Target)
		{
			return EStateTreeRunStatus::Running;
		}
	}

	const float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), Target->GetActorLocation());
	const bool bArrivedByDistance = DistSq <= FMath::Square(InstanceData.AcceptanceRadius + 50.0f);
	const bool bArrivedByMoveStatus = (InstanceData.Controller->GetMoveStatus() == EPathFollowingStatus::Idle);

	if (bArrivedByDistance || bArrivedByMoveStatus)
	{
		InstanceData.bIsWaitingAtWaypoint = true;
		InstanceData.Controller->StopMovement();

		const int32 WaypointIdx = InstanceData.GuardContext ? InstanceData.GuardContext->CurrentPatrolIndex : 0;
		const float DefinedWaitTime = Route->GetWaitTimeForWaypoint(WaypointIdx);
		const float FinalWaitTime = (InstanceData.WaitTimeOverride >= 0.0f) ? InstanceData.WaitTimeOverride : DefinedWaitTime;
		InstanceData.CurrentWaitTimer = FMath::Max(0.1f, FinalWaitTime);

		InstanceData.Controller->SetFocalPoint(Target->GetActorLocation() + Target->GetActorForwardVector() * 200.0f, EAIFocusPriority::Gameplay);
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeTask_NpcPatrol::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.Controller)
	{
		InstanceData.Controller->StopMovement();
		InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}
