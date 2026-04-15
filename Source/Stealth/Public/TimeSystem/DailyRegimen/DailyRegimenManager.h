#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "DailyRegimenManager.generated.h"

class UTimeSubsystem;
class UDailyRegimenTask;

UCLASS(Blueprintable)
class STEALTH_API ADailyRegimenManager : public AInfo
{
	GENERATED_BODY()

	/*Delegates*/
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskStarted, UDailyRegimenTask*, DailyRegimenTask);

	UPROPERTY(BlueprintAssignable)
	FOnTaskStarted OnTaskStarted;

	/*Methods*/
public:
	ADailyRegimenManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void PrepareTask(UDailyRegimenTask* TaskToPrepare);
	UFUNCTION()
	void InitializeTask(UDailyRegimenTask* TaskToInitialize);
	UFUNCTION()
	UDailyRegimenTask* GetNextTask();
	UFUNCTION()
	UDailyRegimenTask* GetNextTaskByTime();
	UFUNCTION()
	void SortTasksByStartTime();

	UFUNCTION()
	void DailyRegimenTask_TaskCompleted(UDailyRegimenTask* Task, bool bWasTaskSuccessful);
	UFUNCTION()
	void TimeSubsystemOnTimeChanged(int32 Hour, int32 Minute);

	/*Properties*/
protected:
	UPROPERTY(Instanced, EditAnywhere)
	TArray<UDailyRegimenTask*> DailyRegimenTasks;
	UPROPERTY()
	TSoftObjectPtr<UTimeSubsystem> TimeSubsystem;
	UPROPERTY()
	int32 CurrentTaskIndex = -1;
	UPROPERTY()
	UDailyRegimenTask* CurrentTask = nullptr;
	UPROPERTY()
	UDailyRegimenTask* PendingTask = nullptr;
};
