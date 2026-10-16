#include "Characters/NPCs/AI/StateTree/StateTreeTask_SetNpcStateTags.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FStateTreeTask_SetNpcStateTags::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (const ANpcAiController* Controller = InstanceData.Controller)
	{
		if (UNpcContextComponent* ContextComp = Controller->GetContextComponent())
		{
			if (InstanceData.StateTag.IsValid())
			{
				ContextComp->SetCurrentStateTag(InstanceData.StateTag);
			}
			if (InstanceData.StanceTag.IsValid())
			{
				ContextComp->SetCurrentStanceTag(InstanceData.StanceTag);
			}
			return EStateTreeRunStatus::Succeeded;
		}
	}
	return EStateTreeRunStatus::Failed;
}
