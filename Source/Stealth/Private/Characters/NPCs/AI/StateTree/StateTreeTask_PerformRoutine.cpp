#include "Characters/NPCs/AI/StateTree/StateTreeTask_PerformRoutine.h"
#include "Characters/NPCs/AI/Patrol/PatrolRoute.h"
#include "Characters/NPCs/AI/Patrol/PatrolSubsystem.h"
#include "Characters/NPCs/AI/Schedule/NpcScheduleComponent.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "Characters/NPCs/Guards/GuardNpcContextComponent.h"
#include "Characters/NPCs/NpcAiController.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"

EStateTreeRunStatus FStateTreeTask_PerformRoutine::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
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

	InstanceData.bHasArrivedAtActivity = false;
	InstanceData.bIsWaitingAtWaypoint = false;
	InstanceData.WaypointWaitTimer = 0.0f;
	InstanceData.ActivePatrolRoute = nullptr;

	if (InstanceData.ScheduleComponent)
	{
		if (!InstanceData.ScheduleComponent->GetActiveSlot().ActivityTag.IsValid())
		{
			InstanceData.ScheduleComponent->EvaluateSchedule();
		}

		InstanceData.CurrentActivityTag = InstanceData.ScheduleComponent->GetActiveActivityTag();
		InstanceData.CurrentLocationTag = InstanceData.ScheduleComponent->GetActiveLocationTag();
		InstanceData.TargetActivityPoint = InstanceData.ScheduleComponent->GetClaimedActivityPoint();
	}
	else
	{
		InstanceData.CurrentActivityTag = InstanceData.ExpectedActivityTag;
	}

	AActor* Target = InstanceData.TargetActivityPoint.Get();
	if (Target)
	{
		InstanceData.Controller->MoveToActor(Target, InstanceData.AcceptanceRadius);
		return EStateTreeRunStatus::Running;
	}

	// If no single activity point, check if this is a patrol routine based on location tag
	if (InstanceData.GuardContext)
	{
		InstanceData.ActivePatrolRoute = InstanceData.GuardContext->AssignPatrolRouteForLocation(InstanceData.CurrentLocationTag);
		if (AActor* Waypoint = InstanceData.GuardContext->GetCurrentPatrolPoint())
		{
			InstanceData.Controller->MoveToActor(Waypoint, InstanceData.AcceptanceRadius);
		}
	}
	else if (UPatrolSubsystem* Subsystem = UPatrolSubsystem::Get(Context.GetOwner()))
	{
		const FVector Pos = Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector;
		InstanceData.ActivePatrolRoute = Subsystem->GetPatrolRouteByLocationTag(InstanceData.CurrentLocationTag, Pos);
		if (APatrolRoute* Route = InstanceData.ActivePatrolRoute.Get())
		{
			if (AActor* Waypoint = Route->GetWaypoint(0))
			{
				InstanceData.Controller->MoveToActor(Waypoint, InstanceData.AcceptanceRadius);
			}
		}
	}

	return EStateTreeRunStatus::Running;
}

//TODO: Refactor, move to separate methods, or even better, separate tasks and states
EStateTreeRunStatus FStateTreeTask_PerformRoutine::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	APawn* Pawn = InstanceData.Controller->GetPawn();
	if (!Pawn)
	{
		return EStateTreeRunStatus::Running;
	}

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

	// Dynamic schedule change detection during routine execution
	if (InstanceData.ScheduleComponent)
	{
		const FNpcScheduleSlot& ActiveSlot = InstanceData.ScheduleComponent->GetActiveSlot();
		AActor* ClaimedPoint = InstanceData.ScheduleComponent->GetClaimedActivityPoint();

		const bool bActivityChanged = (ActiveSlot.ActivityTag != InstanceData.CurrentActivityTag);
		const bool bLocationChanged = (ActiveSlot.LocationTag != InstanceData.CurrentLocationTag);
		const bool bPointChanged = (ClaimedPoint != InstanceData.TargetActivityPoint.Get());

		if (bActivityChanged || bLocationChanged || bPointChanged)
		{
			InstanceData.CurrentActivityTag = ActiveSlot.ActivityTag;
			InstanceData.CurrentLocationTag = ActiveSlot.LocationTag;
			InstanceData.TargetActivityPoint = ClaimedPoint;
			InstanceData.bHasArrivedAtActivity = false;
			InstanceData.bIsWaitingAtWaypoint = false;
			InstanceData.WaypointWaitTimer = 0.0f;
			InstanceData.ActivePatrolRoute = nullptr;

			InstanceData.Controller->StopMovement();
			InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);

			AActor* NewTarget = InstanceData.TargetActivityPoint.Get();
			if (NewTarget)
			{
				InstanceData.Controller->MoveToActor(NewTarget, InstanceData.AcceptanceRadius);
				return EStateTreeRunStatus::Running;
			}

			if (InstanceData.GuardContext)
			{
				InstanceData.ActivePatrolRoute = InstanceData.GuardContext->AssignPatrolRouteForLocation(InstanceData.CurrentLocationTag);
				if (AActor* Waypoint = InstanceData.GuardContext->GetCurrentPatrolPoint())
				{
					InstanceData.Controller->MoveToActor(Waypoint, InstanceData.AcceptanceRadius);
				}
			}
			else if (UPatrolSubsystem* Subsystem = UPatrolSubsystem::Get(Context.GetOwner()))
			{
				const FVector Pos = Pawn->GetActorLocation();
				InstanceData.ActivePatrolRoute = Subsystem->GetPatrolRouteByLocationTag(InstanceData.CurrentLocationTag, Pos);
				if (APatrolRoute* Route = InstanceData.ActivePatrolRoute.Get())
				{
					if (AActor* Waypoint = Route->GetWaypoint(0))
					{
						InstanceData.Controller->MoveToActor(Waypoint, InstanceData.AcceptanceRadius);
					}
				}
			}
			return EStateTreeRunStatus::Running;
		}
	}

	// 1. Single Activity Point Routine (bed, table, chair, guard spot)
	AActor* Target = InstanceData.TargetActivityPoint.Get();
	if (Target)
	{
		if (!InstanceData.bHasArrivedAtActivity)
		{
			const float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), Target->GetActorLocation());
			const bool bArrivedByDistance = DistSq <= FMath::Square(InstanceData.AcceptanceRadius + 50.0f);
			const bool bArrivedByMoveStatus = (InstanceData.Controller->GetMoveStatus() == EPathFollowingStatus::Idle);

			if (bArrivedByDistance || bArrivedByMoveStatus)
			{
				InstanceData.bHasArrivedAtActivity = true;
				InstanceData.Controller->StopMovement();
				InstanceData.Controller->SetFocalPoint(Target->GetActorLocation() + Target->GetActorForwardVector() * 200.0f, EAIFocusPriority::Gameplay);
			}
		}
		return EStateTreeRunStatus::Running;
	}

	// 2. Patrol Route Traversal
	if (APatrolRoute* Route = InstanceData.ActivePatrolRoute.Get())
	{
		if (InstanceData.bIsWaitingAtWaypoint)
		{
			InstanceData.WaypointWaitTimer -= DeltaTime;
			if (InstanceData.WaypointWaitTimer <= 0.0f)
			{
				InstanceData.bIsWaitingAtWaypoint = false;
				if (InstanceData.GuardContext)
				{
					InstanceData.GuardContext->IncrementPatrolIndex();
					if (AActor* NextWaypoint = InstanceData.GuardContext->GetCurrentPatrolPoint())
					{
						InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
						InstanceData.Controller->MoveToActor(NextWaypoint, InstanceData.AcceptanceRadius);
					}
				}
				else
				{
					bool DummyForward = true;
					const int32 NextIdx = Route->GetNextWaypointIndex(0, DummyForward);
					if (AActor* NextWaypoint = Route->GetWaypoint(NextIdx))
					{
						InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
						InstanceData.Controller->MoveToActor(NextWaypoint, InstanceData.AcceptanceRadius);
					}
				}
			}
			return EStateTreeRunStatus::Running;
		}

		AActor* CurrentWaypoint = InstanceData.GuardContext ? InstanceData.GuardContext->GetCurrentPatrolPoint() : Route->GetWaypoint(0);
		if (CurrentWaypoint)
		{
			const float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), CurrentWaypoint->GetActorLocation());
			const bool bArrivedByDistance = DistSq <= FMath::Square(InstanceData.AcceptanceRadius + 50.0f);
			const bool bArrivedByMoveStatus = (InstanceData.Controller->GetMoveStatus() == EPathFollowingStatus::Idle);

			if (bArrivedByDistance || bArrivedByMoveStatus)
			{
				InstanceData.bIsWaitingAtWaypoint = true;
				InstanceData.Controller->StopMovement();

				const int32 WaypointIdx = InstanceData.GuardContext ? InstanceData.GuardContext->CurrentPatrolIndex : 0;
				InstanceData.WaypointWaitTimer = FMath::Max(0.1f, Route->GetWaitTimeForWaypoint(WaypointIdx));
				InstanceData.Controller->SetFocalPoint(CurrentWaypoint->GetActorLocation() + CurrentWaypoint->GetActorForwardVector() * 200.0f, EAIFocusPriority::Gameplay);
			}
		}
		return EStateTreeRunStatus::Running;
	}

	// 3. Fallback: If neither TargetActivityPoint nor ActivePatrolRoute were found at start, retry resolving route
	if (InstanceData.ScheduleComponent)
	{
		if (InstanceData.GuardContext)
		{
			InstanceData.ActivePatrolRoute = InstanceData.GuardContext->AssignPatrolRouteForLocation(InstanceData.CurrentLocationTag);
			if (AActor* Waypoint = InstanceData.GuardContext->GetCurrentPatrolPoint())
			{
				InstanceData.Controller->MoveToActor(Waypoint, InstanceData.AcceptanceRadius);
			}
		}
		else if (UPatrolSubsystem* Subsystem = UPatrolSubsystem::Get(Context.GetOwner()))
		{
			const FVector Pos = Pawn->GetActorLocation();
			InstanceData.ActivePatrolRoute = Subsystem->GetPatrolRouteByLocationTag(InstanceData.CurrentLocationTag, Pos);
			if (APatrolRoute* Route = InstanceData.ActivePatrolRoute.Get())
			{
				if (AActor* Waypoint = Route->GetWaypoint(0))
				{
					InstanceData.Controller->MoveToActor(Waypoint, InstanceData.AcceptanceRadius);
				}
			}
		}
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeTask_PerformRoutine::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.Controller)
	{
		InstanceData.Controller->StopMovement();
		InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}
