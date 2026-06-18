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

	/*Methods*/
public:
	ADailyRegimenManager();

	UFUNCTION(BlueprintCallable)
	[[nodiscard]] const TArray<UDailyRegimenTask*>& GetDailyRegimenTasks() const { return DailyRegimenTasks; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void PrepareTask(UDailyRegimenTask* TaskToPrepare);
	UFUNCTION()
	void InitializeTask(UDailyRegimenTask* TaskToInitialize);
	UFUNCTION(BlueprintCallable)
	UDailyRegimenTask* GetNextTask();
	UFUNCTION()
	UDailyRegimenTask* GetNextTaskByTime();
	UFUNCTION()
	void SortTasksByStartTime();

	UFUNCTION()
	void DailyRegimenTask_TaskCompleted(UDailyRegimenTask* Task);
	UFUNCTION()
	void TimeSubsystemOnTimeChanged(int32 Hour, int32 Minute);

	/*Properties*/
protected:
	UPROPERTY(Instanced, EditAnywhere)
	TArray<UDailyRegimenTask*> DailyRegimenTasks;

protected:
	UPROPERTY()
	int32 CurrentTaskIndex = -1;
	UPROPERTY()
	UDailyRegimenTask* CurrentTask = nullptr;
	UPROPERTY()
	UDailyRegimenTask* PendingTask = nullptr;
	UPROPERTY()
	bool bCurrentTaskSucceeded = false;
};
