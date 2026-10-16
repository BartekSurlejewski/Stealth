#include "Characters/NPCs/AI/StateTree/StateTreeTask_NpcRequestFocus.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FStateTreeTask_NpcRequestFocus::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	ANpcAiController* Controller = InstanceData.Controller;
	if (!Controller)
	{
		return EStateTreeRunStatus::Failed;
	}

	UNpcFocusComponent* FocusComp = Controller->GetFocusComponent();
	if (!FocusComp)
	{
		return EStateTreeRunStatus::Failed;
	}

	if (InstanceData.bClearFocusOnEnter)
	{
		FocusComp->ClearFocus(InstanceData.Priority);
		return EStateTreeRunStatus::Running;
	}

	FNpcFocusTarget Target;
	Target.FocusTag = InstanceData.FocusTag;
	Target.Priority = InstanceData.Priority;
	Target.FocusActor = InstanceData.FocusActor;
	Target.FocusLocation = InstanceData.FocusLocation;
	Target.bHasLocation = InstanceData.bHasLocation;
	Target.Duration = InstanceData.Duration;
	Target.AwarenessReductionMultiplier = InstanceData.AwarenessReductionMultiplier;

	FocusComp->RequestFocus(Target);
	return EStateTreeRunStatus::Running;
}

void FStateTreeTask_NpcRequestFocus::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.bClearFocusOnExit && InstanceData.Controller)
	{
		if (UNpcFocusComponent* FocusComp = InstanceData.Controller->GetFocusComponent())
		{
			FocusComp->ClearFocus(InstanceData.Priority);
		}
	}
}
