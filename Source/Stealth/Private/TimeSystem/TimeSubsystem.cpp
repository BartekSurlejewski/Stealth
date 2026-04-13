#include "TimeSystem/TimeSubsystem.h"

void UTimeSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!Data.bIsPaused)
	{
		Data.TotalElapsedSeconds += DeltaTime * Data.TIME_SCALE;
	}
}

float UTimeSubsystem::GetTimeOfDaySeconds() const
{
	return FMath::Fmod(Data.TotalElapsedSeconds, Data.SECONDS_PER_DAY);
}

float UTimeSubsystem::GetTimeOfDayNormalized() const
{
	return GetTimeOfDaySeconds() / Data.SECONDS_PER_DAY;
}

int32 UTimeSubsystem::GetCurrentDay() const
{
	return static_cast<int32>(Data.TotalElapsedSeconds / Data.SECONDS_PER_DAY);
}

void UTimeSubsystem::GetClockTime(int32& OutHour, int32& OutMinute, int32& OutSecond) const
{
	const int32 TimeOfDaySeconds = static_cast<int32>(GetTimeOfDaySeconds());
	OutHour = TimeOfDaySeconds / 3600;
	OutMinute = (TimeOfDaySeconds % 3600) / 60;
	OutSecond = TimeOfDaySeconds % 60;
}
