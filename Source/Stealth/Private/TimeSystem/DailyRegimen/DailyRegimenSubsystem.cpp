#include "TimeSystem/DailyRegimen/DailyRegimenSubsystem.h"

#include "Core/Data/StealthWorldSettings.h"
#include "Engine/World.h"
#include "Messages/StealthMessages.h"
#include "TimeSystem/TimeSubsystem.h"
#include "TimeSystem/DailyRegimen/DailyRegimenTasks/DailyRegimenTask.h"

class UGameplayMessageSubsystem;

void UDailyRegimenSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	TimeChangedListenerHandle = MsgSubsystem.RegisterListener<FTimeChangedMessage>(FGameplayTag::RequestGameplayTag("Message.Time.TimeChanged"), this,
	                                                                               &UDailyRegimenSubsystem::OnTimeChanged);

	if (!WorldSettings)
	{
		return;
	}
	BuildWorkingCopiesFromSettings();

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

void UDailyRegimenSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(TimeChangedListenerHandle);

	Super::OnWorldEndPlay(InWorld);
}

void UDailyRegimenSubsystem::PrepareTask(UDailyRegimenTask* TaskToPrepare)
{
	if (!TaskToPrepare)
	{
		return;
	}

	int32 CurrentDayTimeAsMinutes = GetWorld()->GetSubsystem<UTimeSubsystem>()->GetCurrentDayTimeAsMinutes();

	if (TaskToPrepare->IsActiveAtTime(CurrentDayTimeAsMinutes))
	{
		InitializeTask(TaskToPrepare);
		return;
	}

	// Wait for proper time to initialize task
	PendingTask = TaskToPrepare;
}

void UDailyRegimenSubsystem::InitializeTask(UDailyRegimenTask* TaskToInitialize)
{
	if (!TaskToInitialize)
	{
		return;
	}

	CurrentTaskIndex = DailyRegimenTasks.Find(TaskToInitialize);

	bCurrentTaskSucceeded = false;
	CurrentTask = TaskToInitialize;
	CurrentTask->InitializeTask();
	CurrentTask->OnTaskCompleted.AddDynamic(this, &UDailyRegimenSubsystem::DailyRegimenTask_TaskCompleted);

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	FDailyTaskStartedMessage Message(CurrentTask);
	MsgSubsystem.BroadcastMessage(FGameplayTag::RequestGameplayTag("Message.DailyTask.OnDailyTaskStarted"), Message);
}

UDailyRegimenTask* UDailyRegimenSubsystem::GetNextTaskByIndex()
{
	int32 NextTaskIndex = CurrentTaskIndex + 1;
	if (NextTaskIndex >= DailyRegimenTasks.Num() || NextTaskIndex < 0)
	{
		NextTaskIndex = 0;
	}

	return DailyRegimenTasks[NextTaskIndex];
}

UDailyRegimenTask* UDailyRegimenSubsystem::GetNextTaskByTime()
{
	int32 CurrentDayTimeAsMinutes = GetWorld()->GetSubsystem<UTimeSubsystem>()->GetCurrentDayTimeAsMinutes();
	for (UDailyRegimenTask* Task : DailyRegimenTasks)
	{
		if (Task->GetStartTimeAsMinutes() >= CurrentDayTimeAsMinutes)
		{
			return Task;
		}
	}

	return nullptr;
}

void UDailyRegimenSubsystem::SortTasksByStartTime()
{
	DailyRegimenTasks.RemoveAll([](const UDailyRegimenTask* Task) { return Task == nullptr; });
	DailyRegimenTasks.Sort([](const UDailyRegimenTask& A, const UDailyRegimenTask& B)
	{
		return A.GetStartTimeAsMinutes() < B.GetStartTimeAsMinutes();
	});
}

void UDailyRegimenSubsystem::BuildWorkingCopiesFromSettings()
{
	if (!WorldSettings)
	{
		return;
	}

	DailyRegimenTasks.Reset();
	DailyRegimenTasks.Reserve(WorldSettings->GetDailyRegimenTasks().Num());

	for (UDailyRegimenTask* SettingsTask : WorldSettings->GetDailyRegimenTasks())
	{
		if (!IsValid(SettingsTask))
		{
			continue;
		}

		UDailyRegimenTask* WorkingCopy = DuplicateObject<UDailyRegimenTask>(SettingsTask, this);
		DailyRegimenTasks.Add(WorkingCopy);
	}
}

void UDailyRegimenSubsystem::ResetTasksToSettings()
{
	if (!WorldSettings)
	{
		return;
	}

	CurrentTaskIndex = -1;
	CurrentTask = nullptr;
	PendingTask = nullptr;
	bCurrentTaskSucceeded = false;

	// Old working copies are just dropped from the array; GC will collect them
	// since nothing else should be holding references.
	BuildWorkingCopiesFromSettings();
	SortTasksByStartTime();
}

void UDailyRegimenSubsystem::DailyRegimenTask_TaskCompleted(UDailyRegimenTask* Task)
{
	Task->OnTaskCompleted.RemoveAll(this);
	bCurrentTaskSucceeded = true;
}

void UDailyRegimenSubsystem::OnTimeChanged(FGameplayTag Channel, const FTimeChangedMessage& Message)
{
	int32 CurrentDayTimeAsMinutes = GetWorld()->GetSubsystem<UTimeSubsystem>()->GetCurrentDayTimeAsMinutes();

	if (CurrentTask && !CurrentTask->IsActiveAtTime(CurrentDayTimeAsMinutes))
	{
		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
		FDailyTaskEndedMessage TaskEndedMessage(CurrentTask, bCurrentTaskSucceeded);
		MsgSubsystem.BroadcastMessage(FGameplayTag::RequestGameplayTag("Message.DailyTask.OnDailyTaskEnded"), TaskEndedMessage);

		CurrentTask->DisposeTask();
		CurrentTask = nullptr;

		PrepareTask(GetNextTaskByIndex());
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
