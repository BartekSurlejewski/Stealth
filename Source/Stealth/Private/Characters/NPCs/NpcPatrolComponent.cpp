#include "Characters/NPCs/NpcPatrolComponent.h"

#include "AI/Controllers/NpcAiController.h"
#include "Characters/NPCs/NpcCharacter.h"
#include "Tasks/AITask_MoveTo.h"

UNpcPatrolComponent::UNpcPatrolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// void UNpcPatrolComponent::BeginPlay()
// {
// 	Super::BeginPlay();
//
// 	ANpcCharacter* OwnerCharacter = Cast<ANpcCharacter>(GetOwner());
// 	if (OwnerCharacter)
// 	{
// 		NpcController = Cast<ANpcAiController>(OwnerCharacter->GetController());
// 	}
//
// 	if (!NpcController)
// 	{
// 		return;
// 	}
//
// 	NpcController->ReceiveMoveCompleted.AddDynamic(this, &UNpcPatrolComponent::OnMoveCompleted);
//
// 	MoveToCurrentTarget();
// }

// void UNpcPatrolComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
// {
// 	Super::EndPlay(EndPlayReason);
//
// 	NpcController->ReceiveMoveCompleted.RemoveDynamic(this, &UNpcPatrolComponent::OnMoveCompleted);
// }

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

// void UNpcPatrolComponent::MoveToCurrentTarget()
// {
// 	if (PatrolTargets.Num() == 0 || !NpcController)
// 	{
// 		return;
// 	}
//
// 	AActor* CurrentTarget = PatrolTargets[CurrentTargetIndex];
// 	if (!CurrentTarget)
// 	{
// 		return;
// 	}
// 	NpcController->MoveToActor(CurrentTarget);
// }

// void UNpcPatrolComponent::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
// {
// 	if (Result != EPathFollowingResult::Success)
// 	{
// 		return;
// 	}
//
// 	GetOwner()->GetWorldTimerManager().SetTimer(PatrolTimer, this, &UNpcPatrolComponent::OnPatrolTimerFinished, 2.0f);
// }
//
// void UNpcPatrolComponent::OnPatrolTimerFinished()
// {
// 	CurrentTargetIndex = (CurrentTargetIndex + 1) % PatrolTargets.Num();
// 	MoveToCurrentTarget();
// }
