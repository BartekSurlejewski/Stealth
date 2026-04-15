#include "TimeSystem/DailyRegimen/DailyRegimenTasks/EnterAreaTask.h"

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

void UEnterAreaTask::TargetArea_OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	OnTaskCompleted.Broadcast(this, true);
}
