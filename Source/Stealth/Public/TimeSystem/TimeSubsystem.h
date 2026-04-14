// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/StealthTickableWorldSubsystem.h"
#include "TimeSubsystem.generated.h"

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
	Day,
	Night
};

USTRUCT()
struct FTimeSubsystemData
{
	GENERATED_BODY()

	static constexpr int32 SECONDS_PER_DAY = 86400; // 24 * 60 * 60
	// How many real seconds = 1 in-game second (e.g. 60 = 1 min real = 1hr game)
	static constexpr float TIME_SCALE = 1200.f;

	UPROPERTY()
	ETimeOfDay CurrentTimeOfDay;
	UPROPERTY()
	bool bIsPaused = false;
	UPROPERTY()
	int32 CurrentDayNum;
	UPROPERTY()
	float SecondsElapsedInDay;
	UPROPERTY()
	float SunriseSeconds;
	UPROPERTY()
	float SunsetSeconds;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, ETimeOfDay, NewTimeOfDay);

UCLASS(Blueprintable)
class STEALTH_API UTimeSubsystem : public UStealthTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnTimeOfDayChanged OnTimeOfDayChanged;

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentDay() const { return Data.CurrentDayNum; }

	UFUNCTION(BlueprintCallable)
	float GetTimeOfDayNormalized() const;
	UFUNCTION(BlueprintCallable)
	float GetTimeOfNightNormalized() const;
	UFUNCTION(BlueprintCallable)
	ETimeOfDay GetCurrentTimeOfDay() const { return Data.CurrentTimeOfDay; };
	UFUNCTION(BlueprintCallable)
	void GetClockTime(int32& OutHour, int32& OutMinute, int32& OutSecond) const;
	UFUNCTION(BlueprintCallable)
	void SetTime(const int32& NewDay, const int32& NewHour, const int32& NewMinute, const int32& NewSecond);

protected:
	UFUNCTION()
	void CheckTimeOfDay();
private:
	UPROPERTY()
	FTimeSubsystemData Data;
};
