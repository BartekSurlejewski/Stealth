#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DailyRegimenTask.generated.h"

class UPrisonerNpcContextComponent;

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class STEALTH_API UDailyRegimenTask : public UObject
{
	GENERATED_BODY()
	/*Delegates*/
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskCompleted, UDailyRegimenTask*, Task);

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTaskCompleted OnTaskCompleted;

	/*Methods*/
public:
	UFUNCTION(BlueprintCallable)
	bool IsInitialized() const { return bIsInitialized; }

	UFUNCTION(BlueprintNativeEvent)
	void InitializeTask();
	virtual void InitializeTask_Implementation();
	UFUNCTION(BlueprintNativeEvent)
	void DisposeTask();
	virtual void DisposeTask_Implementation();
	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetStartTimeAsMinutes() const { return StartHour * 60 + StartMinute; }

	UFUNCTION(BlueprintPure, Category = "Time")
	int32 GetEndTimeAsMinutes() const { return EndHour * 60 + EndMinute; }

	UFUNCTION(BlueprintPure, Category = "Time")
	bool IsActiveAtTime(const int32& DayTimeInMinutes) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void PerformByPrisoner(UPrisonerNpcContextComponent* PrisonerContext);
	virtual void PerformByPrisoner_Implementation(UPrisonerNpcContextComponent* PrisonerContext);

	/*Properties*/
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TaskDescription;

protected:
	UPROPERTY()
	bool bIsInitialized;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta=(ClampMin = "0", ClampMax = "23"))
	int32 StartHour;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta=(ClampMin = "0", ClampMax = "59"))
	int32 StartMinute;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta=(ClampMin = "0", ClampMax = "23"))
	int32 EndHour;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time", meta=(ClampMin = "0", ClampMax = "59"))
	int32 EndMinute;
};
