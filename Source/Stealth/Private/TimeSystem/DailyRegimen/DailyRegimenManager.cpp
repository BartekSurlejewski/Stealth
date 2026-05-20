#include "TimeSystem/DailyRegimen/DailyRegimenManager.h"

#include "TimeSystem/TimeSubsystem.h"
#include "TimeSystem/DailyRegimen/DailyRegimenTasks/DailyRegimenTask.h"


ADailyRegimenManager::ADailyRegimenManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADailyRegimenManager::BeginPlay()
{
	Super::BeginPlay();

	TimeSubsystem = GetWorld()->GetSubsystem<UTimeSubsystem>();
	if (TimeSubsystem)
	{
		TimeSubsystem->OnTimeChanged.AddDynamic(this, &ADailyRegimenManager::TimeSubsystemOnTimeChanged);
	}

	SortTasksByStartTime();
	UDailyRegimenTask* FirstTask;
	if (CurrentTaskIndex >= 0)
	{
		FirstTask = DailyRegimenTasks[CurrentTaskIndex];
	}
	else
	{
		FirstTask = GetNextTaskByTime();
	}

	PrepareTask(FirstTask);
}

void ADailyRegimenManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (TimeSubsystem.IsValid())
	{
		TimeSubsystem->OnTimeChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ADailyRegimenManager::PrepareTask(UDailyRegimenTask* TaskToPrepare)
{
	if (!TaskToPrepare)
	{
		return;
	}
	int32 CurrentDayTimeAsMinutes = TimeSubsystem->GetCurrentDayTimeAsMinutes();

	if (TaskToPrepare->IsActiveAtTime(CurrentDayTimeAsMinutes))
	{
		InitializeTask(TaskToPrepare);
		return;
	}

	// Wait for proper time to initialize task
	PendingTask = TaskToPrepare;
}

void ADailyRegimenManager::InitializeTask(UDailyRegimenTask* TaskToInitialize)
{
	if (!TaskToInitialize)
	{
		return;
	}

	CurrentTaskIndex = DailyRegimenTasks.Find(TaskToInitialize);

	bCurrentTaskSucceeded = false;
	CurrentTask = TaskToInitialize;
	CurrentTask->InitializeTask();
	CurrentTask->OnTaskCompleted.AddDynamic(this, &ADailyRegimenManager::DailyRegimenTask_TaskCompleted);
	OnTaskStarted.Broadcast(CurrentTask);
}

UDailyRegimenTask* ADailyRegimenManager::GetNextTask()
{
	int32 NextTaskIndex = CurrentTaskIndex + 1;
	if (NextTaskIndex >= DailyRegimenTasks.Num() || NextTaskIndex < 0)
	{
		NextTaskIndex = 0;
	}

	return DailyRegimenTasks[NextTaskIndex];
}


UDailyRegimenTask* ADailyRegimenManager::GetNextTaskByTime()
{
	int32 CurrentDayTimeAsMinutes = TimeSubsystem->GetCurrentDayTimeAsMinutes();
	for (UDailyRegimenTask* Task : DailyRegimenTasks)
	{
		if (Task->GetStartTimeAsMinutes() >= CurrentDayTimeAsMinutes)
		{
			return Task;
		}
	}

	return nullptr;
}

void ADailyRegimenManager::SortTasksByStartTime()
{
	DailyRegimenTasks.RemoveAll([](const UDailyRegimenTask* Task) { return Task == nullptr; });
	DailyRegimenTasks.Sort([](const UDailyRegimenTask& A, const UDailyRegimenTask& B)
	{
		return A.GetStartTimeAsMinutes() < B.GetStartTimeAsMinutes();
	});
}

void ADailyRegimenManager::DailyRegimenTask_TaskCompleted(UDailyRegimenTask* Task)
{
	Task->OnTaskCompleted.RemoveAll(this);
	bCurrentTaskSucceeded = true;
}

void ADailyRegimenManager::TimeSubsystemOnTimeChanged(int32 Hour, int32 Minute)
{
	int32 CurrentDayTimeAsMinutes = TimeSubsystem->GetCurrentDayTimeAsMinutes();

	if (CurrentTask && !CurrentTask->IsActiveAtTime(CurrentDayTimeAsMinutes))
	{
		OnTaskEnded.Broadcast(CurrentTask, bCurrentTaskSucceeded);

		CurrentTask->DisposeTask();
		CurrentTask = nullptr;

		PrepareTask(GetNextTask());
		return;
	}

	if (!PendingTask)
	{
		return;
	}

	if (PendingTask->IsActiveAtTime(CurrentDayTimeAsMinutes))
	{
		UDailyRegimenTask* TaskToStart = PendingTask;
		PendingTask = nullptr;
		InitializeTask(TaskToStart);
	}
}
