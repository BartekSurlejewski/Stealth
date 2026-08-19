#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "NpcScheduleAsset.generated.h"

USTRUCT(BlueprintType)
struct FNpcScheduleSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule", meta = (ClampMin = "0", ClampMax = "23"))
	int32 StartHour = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule", meta = (ClampMin = "0", ClampMax = "59"))
	int32 StartMinute = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule", meta = (ClampMin = "0", ClampMax = "23"))
	int32 EndHour = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule", meta = (ClampMin = "0", ClampMax = "59"))
	int32 EndMinute = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
	FGameplayTag ActivityTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
	FGameplayTag LocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
	bool bCanBeInterrupted = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
	FText Description;

	int32 GetStartTimeAsMinutes() const
	{
		return StartHour * 60 + StartMinute;
	}

	int32 GetEndTimeAsMinutes() const
	{
		return EndHour * 60 + EndMinute;
	}

	bool IsActiveAtTime(int32 TimeInMinutes) const
	{
		const int32 Start = GetStartTimeAsMinutes();
		const int32 End = GetEndTimeAsMinutes();

		if (Start <= End)
		{
			return TimeInMinutes >= Start && TimeInMinutes < End;
		}
		// Crosses midnight (e.g., 22:00 -> 06:00)
		return TimeInMinutes >= Start || TimeInMinutes < End;
	}
};

UCLASS(BlueprintType)
class STEALTH_API UNpcScheduleAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
	FGameplayTag ArchetypeTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
	TArray<FNpcScheduleSlot> Slots;

	/** Find active slot for a given world time in minutes */
	UFUNCTION(BlueprintPure, Category = "Schedule")
	bool GetSlotForTime(int32 TimeInMinutes, FNpcScheduleSlot& OutSlot) const;

	/** Find next upcoming slot */
	UFUNCTION(BlueprintPure, Category = "Schedule")
	bool GetNextSlot(int32 TimeInMinutes, FNpcScheduleSlot& OutSlot) const;
};
