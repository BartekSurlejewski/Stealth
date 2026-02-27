#include "Characters/NPCs/NpcPatrolComponent.h"

#include "AI/Controllers/NpcAiController.h"
#include "Characters/NPCs/NpcCharacter.h"
#include "Tasks/AITask_MoveTo.h"

UNpcPatrolComponent::UNpcPatrolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNpcPatrolComponent::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	if (Result != EPathFollowingResult::Success) return;

	CurrentTargetIndex = (CurrentTargetIndex + 1) % PatrolTargets.Num();
	MoveToCurrentTarget();
}

void UNpcPatrolComponent::MoveToCurrentTarget()
{
	if (PatrolTargets.Num() == 0 || !NpcController)
	{
		return;
	}

	AActor* CurrentTarget = PatrolTargets[CurrentTargetIndex];
	if (!CurrentTarget)
	{
		return;
	}
	NpcController->MoveToActor(CurrentTarget);
}


void UNpcPatrolComponent::BeginPlay()
{
	Super::BeginPlay();

	ANpcCharacter* OwnerCharacter = Cast<ANpcCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		NpcController = Cast<ANpcAiController>(OwnerCharacter->GetController());
	}

	if (!NpcController)
	{
		return;
	}

	NpcController->ReceiveMoveCompleted.AddDynamic(this, &UNpcPatrolComponent::OnMoveCompleted);

	MoveToCurrentTarget();
}

void UNpcPatrolComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	NpcController->ReceiveMoveCompleted.RemoveDynamic(this, &UNpcPatrolComponent::OnMoveCompleted);
}


void UNpcPatrolComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
