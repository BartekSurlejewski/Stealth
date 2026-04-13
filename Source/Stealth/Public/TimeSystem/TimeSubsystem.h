// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/StealthTickableWorldSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimeSubsystem.generated.h"

USTRUCT()
struct FTimeSubsystemData
{
	GENERATED_BODY()

	static constexpr int32 SECONDS_PER_DAY = 86400; // 24 * 60 * 60
	static constexpr float START_TIME_OF_DAY = 28800.0f; // 08:00 AM in seconds
	// How many real seconds = 1 in-game second (e.g. 60 = 1 min real = 1hr game)
	static constexpr float TIME_SCALE = 60.f;

	// Total elapsed in-game seconds since day 0, 00:00
	// This is the SINGLE SOURCE OF TRUTH — derive everything else from it
	UPROPERTY()
	float TotalElapsedSeconds = 0.f;
	// Whether time is currently advancing
	UPROPERTY()
	bool bIsPaused = false;
};

UCLASS(Blueprintable)
class STEALTH_API UTimeSubsystem : public UStealthTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	float GetTimeOfDaySeconds() const;
	float GetTimeOfDayNormalized() const;
	int32 GetCurrentDay() const;
	UFUNCTION(BlueprintCallable)
	void GetClockTime(int32& OutHour, int32& OutMinute, int32& OutSecond) const;

private:
	UPROPERTY()
	FTimeSubsystemData Data;
};
