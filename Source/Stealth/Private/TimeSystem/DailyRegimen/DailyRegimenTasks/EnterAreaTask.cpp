#include "TimeSystem/DailyRegimen/DailyRegimenTasks/EnterAreaTask.h"

#include "Characters/NPCs/Prisoners/PrisonerNpcContextComponent.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Engine/TriggerVolume.h"

void UEnterAreaTask::InitializeTask_Implementation()
{
	Super::InitializeTask_Implementation();

	if (!TargetArea)
	{
		return;
	}

	TargetArea->OnActorBeginOverlap.AddDynamic(this, &UEnterAreaTask::TargetArea_OnActorBeginOverlap);
}

void UEnterAreaTask::DisposeTask_Implementation()
{
	Super::DisposeTask_Implementation();

	TargetArea->OnActorBeginOverlap.RemoveAll(this);
}

void UEnterAreaTask::PerformByPrisoner_Implementation(UPrisonerNpcContextComponent* PrisonerContext)
{
	Super::PerformByPrisoner_Implementation(PrisonerContext);
	PrisonerContext->MoveToPosition(TargetArea->GetActorLocation(), [this](bool bSuccess)
	{
#if WITH_EDITOR
		if (!bSuccess)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to move to area"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Moved to target area!"));
		}
#endif
	});
}

bool UEnterAreaTask::IsPlayerPerformingTask_Implementation()
{
	return bHasPlayerEnteredArea;
}


void UEnterAreaTask::TargetArea_OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (AStealthPlayerCharacter* OverlappingStealthCharacter = Cast<AStealthPlayerCharacter>(OtherActor))
	{
		bHasPlayerEnteredArea = true;
		OnTaskCompleted.Broadcast(this);
	}
}
