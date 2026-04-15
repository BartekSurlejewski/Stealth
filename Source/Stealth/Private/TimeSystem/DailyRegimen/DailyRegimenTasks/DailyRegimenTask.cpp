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

void UDailyRegimenTask::FailTask() { OnTaskCompleted.Broadcast(this, false); }

bool UDailyRegimenTask::CanStartAtTime(const int32& DayTimeInMinutes) const
{
	return DayTimeInMinutes >= GetStartTimeAsMinutes() && DayTimeInMinutes < GetEndTimeAsMinutes();
}
