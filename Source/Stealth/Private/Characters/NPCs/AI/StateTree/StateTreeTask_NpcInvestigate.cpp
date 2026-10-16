#include "Characters/NPCs/AI/StateTree/StateTreeTask_NpcInvestigate.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/AI/Perception/NpcAwarenessComponent.h"
#include "StateTreeExecutionContext.h"
#include "Navigation/PathFollowingComponent.h"
#include "GameFramework/Pawn.h"

EStateTreeRunStatus FStateTreeTask_NpcInvestigate::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	ANpcAiController* Controller = InstanceData.Controller;
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	InstanceData.CurrentInvestigationTime = 0.0f;
	InstanceData.bHasReachedLocation = false;

	if (InstanceData.InvestigateLocation.IsZero())
	{
		if (const UNpcFocusComponent* FocusComp = Controller->GetFocusComponent())
		{
			const FNpcFocusTarget& CurrentFocus = FocusComp->GetCurrentFocus();
			if (CurrentFocus.IsValid() && CurrentFocus.bHasLocation)
			{
				InstanceData.InvestigateLocation = CurrentFocus.FocusLocation;
			}
		}

		if (InstanceData.InvestigateLocation.IsZero())
		{
			if (const UNpcAwarenessComponent* AwarenessComp = Controller->GetAwarenessComponent())
			{
				const FNpcAwarenessState& AwarenessState = AwarenessComp->GetAwarenessState();
				if (AwarenessState.bHasInvestigationLocation)
				{
					InstanceData.InvestigateLocation = AwarenessState.InvestigationLocation;
				}
			}
		}
	}

	if (!InstanceData.InvestigateLocation.IsZero())
	{
		Controller->MoveToLocation(InstanceData.InvestigateLocation, InstanceData.AcceptanceRadius);
		return EStateTreeRunStatus::Running;
	}

	return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FStateTreeTask_NpcInvestigate::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	ANpcAiController* Controller = InstanceData.Controller;
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	APawn* Pawn = Controller->GetPawn();
	if (!Pawn)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (!InstanceData.bHasReachedLocation)
	{
		const float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), InstanceData.InvestigateLocation);
		const bool bArrivedByDistance = DistSq <= FMath::Square(InstanceData.AcceptanceRadius + 50.0f);
		const bool bArrivedByMoveStatus = (Controller->GetMoveStatus() == EPathFollowingStatus::Idle);

		if (bArrivedByDistance || bArrivedByMoveStatus)
		{
			InstanceData.bHasReachedLocation = true;
			Controller->StopMovement();
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
	if (ANpcAiController* Controller = InstanceData.Controller)
	{
		Controller->StopMovement();
		Controller->ClearFocus(EAIFocusPriority::Gameplay);

		if (UNpcFocusComponent* FocusComp = Controller->GetFocusComponent())
		{
			FocusComp->ClearFocus(ENpcFocusPriority::MajorDisturbance);
		}
	}
}
