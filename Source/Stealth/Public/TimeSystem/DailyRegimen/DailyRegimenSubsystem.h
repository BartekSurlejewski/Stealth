#pragma once

#include "CoreMinimal.h"
#include "Core/StealthWorldSubsystem.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "DailyRegimenSubsystem.generated.h"

struct FTimeChangedMessage;
class UDailyRegimenTask;

UCLASS()
class STEALTH_API UDailyRegimenSubsystem : public UStealthWorldSubsystem
{
	GENERATED_BODY()

	/*Methods*/
public:
	static UDailyRegimenSubsystem* Get(const UObject* WorldContextObject)
	{
		return UStealthWorldSubsystem::Get<UDailyRegimenSubsystem>(WorldContextObject);
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void OnWorldEndPlay(UWorld& InWorld) override;

	UFUNCTION(BlueprintCallable)
	[[nodiscard]] const TArray<UDailyRegimenTask*>& GetDailyRegimenTasks() const { return DailyRegimenTasks; }

	UFUNCTION(BlueprintCallable)
	[[nodiscard]] UDailyRegimenTask* GetCurrentTask() const { return CurrentTask; }
	
	UFUNCTION(BlueprintCallable)
	bool IsPlayerPerformingCurrentTask() const;

private:
	UFUNCTION()
	void PrepareTask(UDailyRegimenTask* TaskToPrepare);
	UFUNCTION()
	void InitializeTask(UDailyRegimenTask* TaskToInitialize);
	UFUNCTION(BlueprintCallable)
	UDailyRegimenTask* GetNextTaskByIndex();
	UFUNCTION()
	UDailyRegimenTask* GetNextTaskByTime();
	UFUNCTION()
	void SortTasksByStartTime();
	UFUNCTION()
	void BuildWorkingCopiesFromSettings();
	UFUNCTION()
	void ResetTasksToSettings();

	UFUNCTION()
	void DailyRegimenTask_TaskCompleted(UDailyRegimenTask* Task) const;
	UFUNCTION()
	void OnTimeChanged(FGameplayTag Channel, const FTimeChangedMessage& Message);

	/*Properties*/

private:
	UPROPERTY()
	TArray<TObjectPtr<UDailyRegimenTask>> DailyRegimenTasks; // working copies, owned by this subsystem
	UPROPERTY()
	int32 CurrentTaskIndex = -1;
	UPROPERTY()
	UDailyRegimenTask* CurrentTask = nullptr;
	UPROPERTY()
	UDailyRegimenTask* PendingTask = nullptr;

	FGameplayMessageListenerHandle TimeChangedListenerHandle;
};
