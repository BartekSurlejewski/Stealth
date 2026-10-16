#include "Characters/NPCs/AI/StateTree/StateTreeTask_ApplyPerceptionTuning.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/AI/Perception/NpcAwarenessComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FStateTreeTask_ApplyPerceptionTuning::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (const ANpcAiController* Controller = InstanceData.Controller)
	{
		if (UNpcAwarenessComponent* AwarenessComp = Controller->GetAwarenessComponent())
		{
			AwarenessComp->SetPerceptionTuning(InstanceData.TuningParams);
			return EStateTreeRunStatus::Succeeded;
		}
	}
	return EStateTreeRunStatus::Failed;
}
