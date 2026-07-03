#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "StealthWorldSettings.generated.h"

class ATargetPoint;
class UDailyRegimenTask;
class ARuntimeDataContainer;

UCLASS()
class STEALTH_API AStealthWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stealth")
	bool bIsGameplayWorld;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stealth|Time", meta = (ClampMin = "1"))
	int32 StartDay = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stealth|Time", meta=(ClampMin = "0", ClampMax = "23"))
	int32 StartHour;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stealth|Time", meta=(ClampMin = "0", ClampMax = "59"))
	int32 StartMinute;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stealth|Time", meta=(ClampMin = "0", ClampMax = "23"))
	int32 SunriseHour = 6;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stealth|Time", meta=(ClampMin = "0", ClampMax = "23"))
	int32 SunsetHour = 20;

	UPROPERTY(Instanced, EditAnywhere, Category = "Stealth|Daily Regimen")
	TArray<TObjectPtr<UDailyRegimenTask>> DailyRegimenTasks;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stealth|Environment")
	TArray<TObjectPtr<ATargetPoint>> PointsOfInterest;

public:
	UFUNCTION(BlueprintCallable)
	bool IsGameplayWorld() const { return bIsGameplayWorld; }

	UFUNCTION(BlueprintCallable)
	void GetStartDayTime(int32& OutDay, int32& OutHour, int32& OutMinute) const
	{
		OutDay = StartDay;
		OutHour = StartHour;
		OutMinute = StartMinute;
	}

	UFUNCTION(BlueprintCallable)
	[[nodiscard]] int32 GetSunriseHour() const { return SunriseHour; }

	UFUNCTION(BlueprintCallable)
	[[nodiscard]] int32 GetSunsetHour() const { return SunsetHour; }

	UFUNCTION(BlueprintCallable)
	[[nodiscard]] const TArray<UDailyRegimenTask*>& GetDailyRegimenTasks() const { return DailyRegimenTasks; }

	UFUNCTION(BlueprintCallable)
	[[nodiscard]] const TArray<ATargetPoint*>& GetPointsOfInterest() const { return PointsOfInterest; }
};
