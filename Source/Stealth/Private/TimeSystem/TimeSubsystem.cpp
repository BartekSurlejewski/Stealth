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

	Data.SunriseSeconds = (WorldSettings->GetSunriseHour() * 3600);
	Data.SunsetSeconds = (WorldSettings->GetSunsetHour() * 3600);
}

void UTimeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Data.bIsPaused)
	{
		return;
	}
	Data.SecondsElapsedInDay += DeltaTime * Data.TIME_SCALE;

	if (Data.SecondsElapsedInDay >= Data.SECONDS_PER_DAY)
	{
		Data.SecondsElapsedInDay -= Data.SECONDS_PER_DAY;
		Data.CurrentDayNum++;

		//TODO: Add onNewDay event
	}

	CheckTimeOfDay();
}

float UTimeSubsystem::GetTimeOfDayNormalized() const
{
	if (Data.SecondsElapsedInDay <= Data.SunriseSeconds || Data.SecondsElapsedInDay >= Data.SunsetSeconds)
	{
		return -1.0f;
	}
	return FMath::Clamp((Data.SecondsElapsedInDay - Data.SunriseSeconds) / (Data.SunsetSeconds - Data.SunriseSeconds), 0.f, 1.f);
}

float UTimeSubsystem::GetTimeOfNightNormalized() const
{
	const float T = Data.SecondsElapsedInDay;
	const float Rise = Data.SunsetSeconds;
	const float Set = Data.SunriseSeconds + Data.SECONDS_PER_DAY; // next day's sunrise

	// Remap night window [Sunset, NextSunrise] → [0, 1]
	// Handle wraparound (T might be before midnight, so shift it)
	const float AdjustedT = (T < Data.SunriseSeconds) ? (T + Data.SECONDS_PER_DAY) : T;

	return FMath::Clamp((AdjustedT - Rise) / (Set - Rise), 0.f, 1.f);
}

void UTimeSubsystem::GetClockTime(int32& OutHour, int32& OutMinute, int32& OutSecond) const
{
	OutHour = Data.SecondsElapsedInDay / 3600;
	OutMinute = FMath::Fmod(Data.SecondsElapsedInDay, 3600) / 60;
	OutSecond = FMath::Fmod(Data.SecondsElapsedInDay, 60);
}

void UTimeSubsystem::SetTime(const int32& NewDay, const int32& NewHour, const int32& NewMinute, const int32& NewSecond)
{
	Data.CurrentDayNum = NewDay;
	Data.SecondsElapsedInDay = (NewHour * 3600) + (NewMinute * 60) + NewSecond;
}

void UTimeSubsystem::CheckTimeOfDay()
{
	switch (Data.CurrentTimeOfDay)
	{
	case ETimeOfDay::Day:
		{
			if (Data.SecondsElapsedInDay <= Data.SunriseSeconds || Data.SecondsElapsedInDay >= Data.SunsetSeconds)
			{
				Data.CurrentTimeOfDay = ETimeOfDay::Night;
				OnTimeOfDayChanged.Broadcast(ETimeOfDay::Night);

				UE_LOG(LogStealth, Log, TEXT("Time of day changed to night"));
			}
			break;
		}
	case ETimeOfDay::Night:
		{
			if (Data.SecondsElapsedInDay >= Data.SunriseSeconds && Data.SecondsElapsedInDay <= Data.SunsetSeconds)
			{
				Data.CurrentTimeOfDay = ETimeOfDay::Day;
				OnTimeOfDayChanged.Broadcast(ETimeOfDay::Day);

				UE_LOG(LogStealth, Log, TEXT("Time of day changed to day"));
			}
			break;
		}
	}
}
