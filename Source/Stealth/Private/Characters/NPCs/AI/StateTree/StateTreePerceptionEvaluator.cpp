#include "Characters/NPCs/AI/StateTree/StateTreePerceptionEvaluator.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

namespace
{
	UNpcContextComponent* ResolveNpcContextComponent(FStateTreeExecutionContext& Context)
	{
		// TODO: try to remove casting and calling FindComponentByClass
		if (const AActor* OwnerActor = Cast<AActor>(Context.GetOwner()))
		{
			if (UNpcContextComponent* Comp = OwnerActor->FindComponentByClass<UNpcContextComponent>())
			{
				return Comp;
			}

			const AController* Controller = Cast<AController>(OwnerActor);
			if (!Controller)
			{
				return nullptr;
			}

			if (const APawn* Pawn = Controller->GetPawn())
			{
				if (UNpcContextComponent* PawnComp = Pawn->FindComponentByClass<UNpcContextComponent>())
				{
					return PawnComp;
				}
			}
		}
		return nullptr;
	}
}

void FStateTreePerceptionEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.NpcContext)
	{
		InstanceData.NpcContext = ResolveNpcContextComponent(Context);
	}

	Tick(Context, 0.0f);
}

void FStateTreePerceptionEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.NpcContext)
	{
		InstanceData.NpcContext = ResolveNpcContextComponent(Context);
		if (!InstanceData.NpcContext)
		{
			return;
		}
	}

	InstanceData.CurrentStateTag = InstanceData.NpcContext->GetCurrentStateTag();
	InstanceData.Awareness = InstanceData.NpcContext->GetAwareness();
	InstanceData.AlertLevel = InstanceData.NpcContext->GetAlertLevel();
	InstanceData.bHasPlayerLineOfSight = InstanceData.NpcContext->HasPlayerLineOfSight();
	InstanceData.bEffectivelySeesPlayer = InstanceData.NpcContext->EffectivelySeesPlayer();
	InstanceData.bIsPlayerPerformingIllegalAction = InstanceData.NpcContext->IsPlayerPerformingIllegalAction(InstanceData.NpcContext->GetPlayerCharacter());
	InstanceData.LastKnownPlayerPos = InstanceData.NpcContext->GetLastKnownPlayerPos();
	InstanceData.LastHeardSoundLocation = InstanceData.NpcContext->GetLastHeardSoundLocation();
}
