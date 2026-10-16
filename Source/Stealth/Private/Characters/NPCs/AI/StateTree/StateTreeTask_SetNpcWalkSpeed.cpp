#include "Characters/NPCs/AI/StateTree/StateTreeTask_SetNpcWalkSpeed.h"
#include "Characters/NPCs/NpcAiController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FStateTreeTask_SetNpcWalkSpeed::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (const ANpcAiController* Controller = InstanceData.Controller)
	{
		if (const ACharacter* Character = Cast<ACharacter>(Controller->GetPawn()))
		{
			if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
			{
				MoveComp->MaxWalkSpeed = InstanceData.MaxWalkSpeed;
				return EStateTreeRunStatus::Succeeded;
			}
		}
	}
	return EStateTreeRunStatus::Failed;
}
