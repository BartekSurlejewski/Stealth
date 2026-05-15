#include "Characters/NPCs/Guards/NpcPatrolComponent.h"

UNpcPatrolComponent::UNpcPatrolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

AActor* UNpcPatrolComponent::GetCurrentTarget() const
{
	if (PatrolTargets.IsEmpty())
	{
		return nullptr;
	}

	const int32 SafeIndex = CurrentTargetIndex % PatrolTargets.Num();
	return PatrolTargets[SafeIndex];
}

void UNpcPatrolComponent::IncrementTargetIndex()
{
	CurrentTargetIndex = (CurrentTargetIndex + 1) % PatrolTargets.Num();
}

bool UNpcPatrolComponent::IsOnWalkingPatrol() const { return bIsOnWalkingPatrol; }

void UNpcPatrolComponent::SetWalkingPatrol(bool newIsOnWalkingPatrol)
{
	bIsOnWalkingPatrol = newIsOnWalkingPatrol;
}
