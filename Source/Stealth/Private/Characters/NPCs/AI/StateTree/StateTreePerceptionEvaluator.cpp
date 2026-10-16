#include "Characters/NPCs/AI/StateTree/StateTreePerceptionEvaluator.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/AI/Perception/NpcAwarenessComponent.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "StateTreeExecutionContext.h"

void FStateTreePerceptionEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	Tick(Context, 0.0f);
}

void FStateTreePerceptionEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const ANpcAiController* Controller = InstanceData.Controller;
	if (!Controller)
	{
		return;
	}

	if (const UNpcAwarenessComponent* AwarenessComp = Controller->GetAwarenessComponent())
	{
		InstanceData.AwarenessState = AwarenessComp->GetAwarenessState();
	}

	if (const UNpcFocusComponent* FocusComp = Controller->GetFocusComponent())
	{
		InstanceData.CurrentFocus = FocusComp->GetCurrentFocus();
	}

	if (const UNpcProfile* Profile = Controller->GetNpcProfile())
	{
		InstanceData.SuspicionThreshold_Suspicious = Profile->SuspicionThreshold_Suspicious;
		InstanceData.SuspicionThreshold_Alert = Profile->SuspicionThreshold_Alert;
	}
}
