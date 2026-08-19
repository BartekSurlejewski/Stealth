#include "Characters/NPCs/AI/StateTree/StateTreeScheduleEvaluator.h"
#include "Characters/NPCs/AI/Schedule/NpcScheduleComponent.h"
#include "StateTreeExecutionContext.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"

namespace
{
	UNpcScheduleComponent* ResolveScheduleComponent(FStateTreeExecutionContext& Context)
	{
		// TODO: try to remove casting and calling FindComponentByClass
		if (const AActor* OwnerActor = Cast<AActor>(Context.GetOwner()))
		{
			if (UNpcScheduleComponent* Comp = OwnerActor->FindComponentByClass<UNpcScheduleComponent>())
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
				if (UNpcScheduleComponent* PawnComp = Pawn->FindComponentByClass<UNpcScheduleComponent>())
				{
					return PawnComp;
				}
			}
		}
		return nullptr;
	}
}

void FStateTreeScheduleEvaluator::TreeStart(FStateTreeExecutionContext& Context) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.ScheduleComponent)
	{
		InstanceData.ScheduleComponent = ResolveScheduleComponent(Context);
	}

	Tick(Context, 0.0f);
}

void FStateTreeScheduleEvaluator::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	if (!InstanceData.ScheduleComponent)
	{
		InstanceData.ScheduleComponent = ResolveScheduleComponent(Context);
		if (!InstanceData.ScheduleComponent)
		{
			return;
		}
	}

	const FNpcScheduleSlot& Slot = InstanceData.ScheduleComponent->GetActiveSlot();
	InstanceData.ActiveActivityTag = Slot.ActivityTag;
	InstanceData.ActiveLocationTag = Slot.LocationTag;
	InstanceData.TargetActivityPoint = InstanceData.ScheduleComponent->GetClaimedActivityPoint();
	InstanceData.bCanBeInterrupted = Slot.bCanBeInterrupted;
	InstanceData.Priority = Slot.Priority;

	if (AActor* TargetActor = InstanceData.TargetActivityPoint.Get())
	{
		InstanceData.TargetLocation = TargetActor->GetActorLocation();
	}
	else
	{
		InstanceData.TargetLocation = FVector::ZeroVector;
	}
}
