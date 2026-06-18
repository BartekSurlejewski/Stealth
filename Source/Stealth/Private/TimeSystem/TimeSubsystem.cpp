#include "TimeSystem/TimeSubsystem.h"

#include "Core/Data/StealthWorldSettings.h"
#include "Stealth/Stealth.h"

void UTimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!WorldSettings)
	{
		return;
	}

	int32 StartDay;
	int32 StartHour;
	int32 StartMinute;
	WorldSettings->GetStartDayTime(StartDay, StartHour, StartMinute);
	SetTime(StartDay, StartHour, StartMinute, 0);

	Data.DayStartTimeInSeconds = (WorldSettings->GetSunriseHour() * 3600);
	Data.DayEndTimeInSeconds = (WorldSettings->GetSunsetHour() * 3600);

	RegisterCheats();
}

void UTimeSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	ETimeOfDay StartingTimeOfDay = Data.CurrentTimeOfDay;
	CheckTimeOfDay();

	if (StartingTimeOfDay == Data.CurrentTimeOfDay) // Event was already invoked if time of day changed at the start
	{
		OnTimeOfDayChanged.Broadcast(Data.CurrentTimeOfDay);
	}
}

void UTimeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Data.bIsPaused)
	{
		return;
	}
	Data.SecondsElapsedInDay += DeltaTime * Data.IN_GAME_SECONDS_PER_REAL_SECOND * ClockTimeScale;

	if (Data.SecondsElapsedInDay >= Data.SECONDS_PER_DAY)
	{
		Data.SecondsElapsedInDay -= Data.SECONDS_PER_DAY;
		Data.CurrentDay++;

		//TODO: Add onNewDay event
	}

	Data.CurrentHour = Data.SecondsElapsedInDay / 3600;
	Data.CurrentMinute = FMath::Fmod(Data.SecondsElapsedInDay, 3600) / 60;

	if (Data.LastMinute != Data.CurrentMinute)
	{
		Data.LastMinute = Data.CurrentMinute;
		OnTimeChanged.Broadcast(Data.CurrentHour, Data.CurrentMinute);
	}

	CheckTimeOfDay();
}

float UTimeSubsystem::GetTimeOfDayNormalized() const
{
	const float T = Data.SecondsElapsedInDay;
	const float Rise = Data.DayStartTimeInSeconds - 3600/6;
	const float Set = Data.DayEndTimeInSeconds + 3600/6;

	if (T <= Rise || T >= Set)
	{
		return -1.0f;
	}
	return FMath::Clamp((T - Rise) / (Set - Rise), 0.f, 1.f);
}

float UTimeSubsystem::GetTimeOfNightNormalized() const
{
	const float T = Data.SecondsElapsedInDay;
	const float Rise = Data.DayEndTimeInSeconds - 3600 * 1.5f;
	const float Set = Data.DayStartTimeInSeconds + Data.SECONDS_PER_DAY + 3600/6; // next day's sunrise

	// Remap night window [Sunset, NextSunrise] → [0, 1]
	// Handle wraparound (T might be before midnight, so shift it)
	const float AdjustedT = (T < Data.DayStartTimeInSeconds) ? (T + Data.SECONDS_PER_DAY) : T;

	return FMath::Clamp((AdjustedT - Rise) / (Set - Rise), 0.f, 1.f);
}

bool UTimeSubsystem::IsTimeOfDay(ETimeOfDay TimeOfDay) const
{
	return GetCurrentTimeOfDay() == TimeOfDay;
}

void UTimeSubsystem::GetClockTime(int32& OutHour, int32& OutMinute) const
{
	OutHour = Data.CurrentHour;
	OutMinute = Data.CurrentMinute;
}

void UTimeSubsystem::SetTime(const int32& NewDay, const int32& NewHour, const int32& NewMinute, const int32& NewSecond)
{
	Data.CurrentDay = NewDay;
	Data.SecondsElapsedInDay = (NewHour * 3600) + (NewMinute * 60) + NewSecond;
}

void UTimeSubsystem::SetClockTimeScale(const float NewClockTimeScale) { ClockTimeScale = NewClockTimeScale; }

void UTimeSubsystem::CheckTimeOfDay()
{
	switch (Data.CurrentTimeOfDay)
	{
	case ETimeOfDay::Day:
		{
			if (Data.SecondsElapsedInDay <= Data.DayStartTimeInSeconds || Data.SecondsElapsedInDay >= Data.DayEndTimeInSeconds)
			{
				Data.CurrentTimeOfDay = ETimeOfDay::Night;
				OnTimeOfDayChanged.Broadcast(ETimeOfDay::Night);

				UE_LOG(LogStealth, Log, TEXT("Time of day changed to night"));
			}
			break;
		}
	case ETimeOfDay::Night:
		{
			if (Data.SecondsElapsedInDay >= Data.DayStartTimeInSeconds && Data.SecondsElapsedInDay <= Data.DayEndTimeInSeconds)
			{
				Data.CurrentTimeOfDay = ETimeOfDay::Day;
				OnTimeOfDayChanged.Broadcast(ETimeOfDay::Day);

				UE_LOG(LogStealth, Log, TEXT("Time of day changed to day"));
			}
			break;
		}
	}
}


#pragma  region Cheats

void UTimeSubsystem::RegisterCheats()
{
	IConsoleCommand* SetInGameTimeSpeed = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("Cheat.Time.SetClockTimeScale"),
		TEXT("Change in-game clock time scale"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UTimeSubsystem::SetClockTimeScale_Parse),
		ECVF_Cheat);
}

void UTimeSubsystem::SetClockTimeScale_Parse(const TArray<FString>& Args)
{
	float NewClockTimeScale = FCString::Atof(*Args[0]);
	SetClockTimeScale(NewClockTimeScale);
}


#pragma endregion
