#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "StealthWorldSettings.generated.h"

class ARuntimeDataContainer;

UCLASS()
class STEALTH_API AStealthWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stealth")
	TObjectPtr<ARuntimeDataContainer> SceneReferences;

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

public:
	bool IsGameplayWorld() const { return bIsGameplayWorld; }

	void GetStartDayTime(int32& OutDay, int32& OutHour, int32& OutMinute) const
	{
		OutDay = StartDay;
		OutHour = StartHour;
		OutMinute = StartMinute;
	}

	[[nodiscard]] int32 GetSunriseHour() const { return SunriseHour; }
	[[nodiscard]] int32 GetSunsetHour() const { return SunsetHour; }
};
