#include "Characters/NPCs/AI/StateTree/StateTreeTask_NpcInvestigate.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"

EStateTreeRunStatus FStateTreeTask_NpcInvestigate::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
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

	if (!InstanceData.NpcContext && InstanceData.Controller)
	{
		InstanceData.NpcContext = InstanceData.Controller->FindComponentByClass<UNpcContextComponent>();
		if (!InstanceData.NpcContext && InstanceData.Controller->GetPawn())
		{
			InstanceData.NpcContext = InstanceData.Controller->GetPawn()->FindComponentByClass<UNpcContextComponent>();
		}
	}

	InstanceData.CurrentInvestigationTime = 0.0f;
	InstanceData.bHasReachedLocation = false;

	if (InstanceData.InvestigateLocation.IsZero() && InstanceData.NpcContext)
	{
		const FNpcFocusTarget& CurrentFocus = InstanceData.NpcContext->GetCurrentFocus();
		if (CurrentFocus.IsValid() && !CurrentFocus.FocusLocation.IsZero())
		{
			InstanceData.InvestigateLocation = CurrentFocus.FocusLocation;
		}
		else if (!InstanceData.NpcContext->GetLastKnownPlayerPos().IsZero())
		{
			InstanceData.InvestigateLocation = InstanceData.NpcContext->GetLastKnownPlayerPos();
		}
		else if (!InstanceData.NpcContext->GetLastHeardSoundLocation().IsZero())
		{
			InstanceData.InvestigateLocation = InstanceData.NpcContext->GetLastHeardSoundLocation();
		}
	}

	if (InstanceData.Controller && !InstanceData.InvestigateLocation.IsZero())
	{
		InstanceData.Controller->MoveToLocation(InstanceData.InvestigateLocation, InstanceData.AcceptanceRadius);
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FStateTreeTask_NpcInvestigate::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (!InstanceData.Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	APawn* Pawn = InstanceData.Controller->GetPawn();
	if (!Pawn)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.bHasReachedLocation)
	{
		const float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), InstanceData.InvestigateLocation);
		const bool bArrivedByDistance = DistSq <= FMath::Square(InstanceData.AcceptanceRadius + 50.0f);
		const bool bArrivedByMoveStatus = (InstanceData.Controller->GetMoveStatus() == EPathFollowingStatus::Idle);

		if (bArrivedByDistance || bArrivedByMoveStatus)
		{
			InstanceData.bHasReachedLocation = true;
			InstanceData.Controller->StopMovement();
		}
	}

	if (InstanceData.bHasReachedLocation)
	{
		InstanceData.CurrentInvestigationTime += DeltaTime;
		if (InstanceData.CurrentInvestigationTime >= InstanceData.InvestigationDuration)
		{
			return EStateTreeRunStatus::Succeeded;
		}
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeTask_NpcInvestigate::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.Controller)
	{
		InstanceData.Controller->StopMovement();
		InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}

	if (InstanceData.NpcContext)
	{
		InstanceData.NpcContext->ClearFocus(ENpcFocusPriority::MajorDisturbance);
	}
}
