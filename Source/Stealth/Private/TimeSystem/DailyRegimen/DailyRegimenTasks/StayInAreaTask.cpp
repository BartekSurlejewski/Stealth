// 


#include "TimeSystem/DailyRegimen/DailyRegimenTasks/StayInAreaTask.h"

#include "Characters/NPCs/Prisoners/PrisonerNpcContextComponent.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Engine/TriggerVolume.h"

void UStayInAreaTask::InitializeTask_Implementation()
{
	Super::InitializeTask_Implementation();

	if (!TargetArea)
	{
		return;
	}

	TargetArea->OnActorBeginOverlap.AddDynamic(this, &UStayInAreaTask::TargetArea_OnActorBeginOverlap);
	TargetArea->OnActorEndOverlap.AddDynamic(this, &UStayInAreaTask::TargetArea_OnActorEndOverlap);
}

void UStayInAreaTask::DisposeTask_Implementation()
{
	Super::DisposeTask_Implementation();

	TargetArea->OnActorBeginOverlap.RemoveAll(this);
}

void UStayInAreaTask::PerformByPrisoner_Implementation(UPrisonerNpcContextComponent* PrisonerContext)
{
	Super::PerformByPrisoner_Implementation(PrisonerContext);

	//TODO: Rework to make sure prisoner stays in area for the duration of the task
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

void UStayInAreaTask::TargetArea_OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (AStealthPlayerCharacter* OverlappingStealthCharacter = Cast<AStealthPlayerCharacter>(OtherActor))
	{
		bIsPlayerInArea = true;
	}
}

void UStayInAreaTask::TargetArea_OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (AStealthPlayerCharacter* OverlappingStealthCharacter = Cast<AStealthPlayerCharacter>(OtherActor))
	{
		bIsPlayerInArea = false;
	}
}
