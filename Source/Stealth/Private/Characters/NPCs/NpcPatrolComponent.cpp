#include "Characters/NPCs/NpcPatrolComponent.h"

UNpcPatrolComponent::UNpcPatrolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

AActor* UNpcPatrolComponent::GetTarget()
{
	if (PatrolTargets.Num() == 0)
	{
		return nullptr;
	}

	if (CurrentTargetIndex >= PatrolTargets.Num() || CurrentTargetIndex < 0)
	{
		CurrentTargetIndex = 0;
	}

	TObjectPtr<AActor> CurrentTarget = PatrolTargets[CurrentTargetIndex];
	CurrentTargetIndex = (CurrentTargetIndex + 1) % PatrolTargets.Num();

	return CurrentTarget;
}
