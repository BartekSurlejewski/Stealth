#include "Characters/NPCs/AI/StateTree/StateTreeTask_PerformRoutine.h"
#include "Characters/NPCs/AI/Schedule/NpcScheduleComponent.h"
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

	if (!InstanceData.ScheduleComponent && InstanceData.Controller)
	{
		InstanceData.ScheduleComponent = InstanceData.Controller->FindComponentByClass<UNpcScheduleComponent>();
		if (!InstanceData.ScheduleComponent && InstanceData.Controller->GetPawn())
		{
			InstanceData.ScheduleComponent = InstanceData.Controller->GetPawn()->FindComponentByClass<UNpcScheduleComponent>();
		}
	}

	InstanceData.bHasArrivedAtActivity = false;

	if (!InstanceData.TargetActivityPoint.IsValid() && InstanceData.ScheduleComponent)
	{
		InstanceData.TargetActivityPoint = InstanceData.ScheduleComponent->GetClaimedActivityPoint();
	}

	AActor* Target = InstanceData.TargetActivityPoint.Get();
	if (InstanceData.Controller && Target)
	{
		InstanceData.Controller->MoveToActor(Target, InstanceData.AcceptanceRadius);
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeTask_PerformRoutine::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	AActor* Target = InstanceData.TargetActivityPoint.Get();
	APawn* Pawn = InstanceData.Controller->GetPawn();

	if (Target && Pawn && !InstanceData.bHasArrivedAtActivity)
	{
		const float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), Target->GetActorLocation());
		const bool bArrivedByDistance = DistSq <= FMath::Square(InstanceData.AcceptanceRadius + 50.0f);
		const bool bArrivedByMoveStatus = (InstanceData.Controller->GetMoveStatus() == EPathFollowingStatus::Idle);

		if (bArrivedByDistance || bArrivedByMoveStatus)
		{
			InstanceData.bHasArrivedAtActivity = true;
			InstanceData.Controller->StopMovement();
			InstanceData.Controller->SetFocalPoint(Target->GetActorLocation() + Target->GetActorForwardVector() * 200.0f);
		}
	}

	// Keep running routine state until interrupted or transitioned by schedule evaluator
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
