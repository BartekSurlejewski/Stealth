#include "Characters/NPCs/AI/StateTree/StateTreeTask_NpcRequestFocus.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Pawn.h"

EStateTreeRunStatus FStateTreeTask_NpcRequestFocus::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	AActor* OwnerActor = Cast<AActor>(Context.GetOwner());

	if (!InstanceData.Controller)
	{
		InstanceData.Controller = Cast<ANpcAiController>(OwnerActor);
		if (!InstanceData.Controller && OwnerActor)
		{
			if (const APawn* Pawn = Cast<APawn>(OwnerActor))
			{
				InstanceData.Controller = Cast<ANpcAiController>(Pawn->GetController());
			}
		}
	}

	if (!InstanceData.FocusComponent)
	{
		if (InstanceData.Controller)
		{
			InstanceData.FocusComponent = InstanceData.Controller->GetFocusComponent();
		}
		if (!InstanceData.FocusComponent && OwnerActor)
		{
			InstanceData.FocusComponent = OwnerActor->FindComponentByClass<UNpcFocusComponent>();
		}
	}

	if (!InstanceData.FocusComponent)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.bClearFocusOnEnter)
	{
		InstanceData.FocusComponent->ClearFocus(InstanceData.Priority);
		return EStateTreeRunStatus::Running;
	}

	FNpcFocusTarget Target;
	Target.FocusTag = InstanceData.FocusTag;
	Target.Priority = InstanceData.Priority;
	Target.FocusActor = InstanceData.FocusActor;
	Target.FocusLocation = InstanceData.FocusLocation;
	Target.Duration = InstanceData.Duration;

	InstanceData.FocusComponent->RequestFocus(Target);
	return EStateTreeRunStatus::Running;
}

void FStateTreeTask_NpcRequestFocus::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.bClearFocusOnExit && InstanceData.FocusComponent)
	{
		InstanceData.FocusComponent->ClearFocus(InstanceData.Priority);
	}
}
