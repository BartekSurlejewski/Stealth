// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeSystem/DailyRegimen/DailyRegimenTasks/DailyRegimenTask.h"

#include "Stealth/Stealth.h"

void UDailyRegimenTask::InitializeTask_Implementation()
{
	UE_LOG(LogStealth, Warning, TEXT("Initializing DailyRegimenTask %s"), *TaskDescription.ToString())
	bIsInitialized = true;
}

void UDailyRegimenTask::DisposeTask_Implementation()
{
	UE_LOG(LogStealth, Warning, TEXT("Disposing DailyRegimenTask %s"), *TaskDescription.ToString())
	bIsInitialized = false;
}

bool UDailyRegimenTask::IsActiveAtTime(const int32& DayTimeInMinutes) const
{
	const int32 StartTimeAsMinutes = GetStartTimeAsMinutes();
	const int32 EndTimeAsMinutes = GetEndTimeAsMinutes();

	if (StartTimeAsMinutes <= EndTimeAsMinutes)
	{
		// Normal same-day task: [Start -------- End]
		// e.g. 08:00 - 17:00
		return DayTimeInMinutes >= StartTimeAsMinutes && DayTimeInMinutes < EndTimeAsMinutes;
	}
	else
	{
		// Overnight task: [---- End] ... [Start ----]
		// e.g. 20:00 - 06:00 → active if time >= 20:00 OR time < 06:00
		return DayTimeInMinutes >= StartTimeAsMinutes || DayTimeInMinutes < EndTimeAsMinutes;
	}
}
