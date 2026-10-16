#include "Characters/NPCs/AI/StateTree/StateTreeTask_PlayEnterMontage.h"
#include "Characters/NPCs/NpcAiController.h"
#include "GameFramework/Character.h"
#include "Animation/AnimMontage.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FStateTreeTask_PlayEnterMontage::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (InstanceData.EnterStateMontage && InstanceData.Controller)
	{
		if (ACharacter* Character = Cast<ACharacter>(InstanceData.Controller->GetPawn()))
		{
			Character->PlayAnimMontage(InstanceData.EnterStateMontage);
			return EStateTreeRunStatus::Succeeded;
		}
	}
	return EStateTreeRunStatus::Succeeded;
}
