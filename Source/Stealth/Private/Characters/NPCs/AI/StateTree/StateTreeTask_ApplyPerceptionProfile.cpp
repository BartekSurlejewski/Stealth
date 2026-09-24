#include "Characters/NPCs/AI/StateTree/StateTreeTask_ApplyPerceptionProfile.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/AI/Perception/NpcAwarenessComponent.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimMontage.h"

EStateTreeRunStatus FStateTreeTask_ApplyPerceptionProfile::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
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

	APawn* Pawn = nullptr;
	if (InstanceData.Controller)
	{
		Pawn = InstanceData.Controller->GetPawn();
	}
	else
	{
		Pawn = Cast<APawn>(Context.GetOwner());
	}

	if (!InstanceData.NpcContext)
	{
		if (InstanceData.Controller)
		{
			InstanceData.NpcContext = InstanceData.Controller->FindComponentByClass<UNpcContextComponent>();
		}
		if (!InstanceData.NpcContext && Pawn)
		{
			InstanceData.NpcContext = Pawn->FindComponentByClass<UNpcContextComponent>();
		}
	}

	if (!InstanceData.AwarenessComponent)
	{
		if (InstanceData.Controller)
		{
			InstanceData.AwarenessComponent = InstanceData.Controller->FindComponentByClass<UNpcAwarenessComponent>();
		}
		if (!InstanceData.AwarenessComponent && Pawn)
		{
			InstanceData.AwarenessComponent = Pawn->FindComponentByClass<UNpcAwarenessComponent>();
		}
	}

	if (Pawn)
	{
		if (UCharacterMovementComponent* MoveComp = Pawn->FindComponentByClass<UCharacterMovementComponent>())
		{
			MoveComp->MaxWalkSpeed = InstanceData.MaxWalkSpeed;
		}

		if (InstanceData.EnterStateMontage)
		{
			if (ACharacter* Character = Cast<ACharacter>(Pawn))
			{
				Character->PlayAnimMontage(InstanceData.EnterStateMontage);
			}
		}
	}

	if (InstanceData.AwarenessComponent)
	{
		InstanceData.AwarenessComponent->SetPerceptionTuningDirect(InstanceData.Mode, InstanceData.SuspicionGainMultiplier, InstanceData.SuspicionDecayMultiplier);
	}

	if (InstanceData.NpcContext)
	{
		if (InstanceData.StateTag.IsValid())
		{
			InstanceData.NpcContext->SetCurrentStateTag(InstanceData.StateTag);
		}
		if (InstanceData.StanceTag.IsValid())
		{
			InstanceData.NpcContext->SetCurrentStanceTag(InstanceData.StanceTag);
		}
	}

	return EStateTreeRunStatus::Running;
}
