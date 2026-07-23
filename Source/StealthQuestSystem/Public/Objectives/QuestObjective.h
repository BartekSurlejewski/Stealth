#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestObjective.generated.h"


class UQuestCondition;

UENUM(BlueprintType)
enum class EQuestObjectiveStatus : uint8
{
	Locked,
	Active,
	Completed,
	Failed,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestObjectiveStatusChanged, FName, ObjectiveId, EQuestObjectiveStatus, NewStatus);

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, Blueprintable)
class STEALTHQUESTSYSTEM_API UQuestObjective : public UObject
{
	GENERATED_BODY()

	/*Events*/
public:
	UPROPERTY(BlueprintAssignable, Category="Quest|Objective")
	FOnQuestObjectiveStatusChanged OnQuestObjectiveStatusChanged;

	/*Methods*/
public:
	UFUNCTION(BlueprintNativeEvent, Category="Quest|Objective")
	void ActivateObjective();
	virtual void ActivateObjective_Implementation();
	UFUNCTION(BlueprintNativeEvent, Category="Quest|Objective")
	void DeactivateObjective();
	virtual void DeactivateObjective_Implementation();

	UFUNCTION()
	void ForceCompleteObjective();

	UFUNCTION()
	EQuestObjectiveStatus GetStatus() const { return Status; }

protected:
	UFUNCTION()
	void SetStatus(EQuestObjectiveStatus NewStatus);

	/*Properties*/
public:
	UPROPERTY(EditDefaultsOnly, Category="Quest|Objective")
	FName ObjectiveID;
	UPROPERTY(EditDefaultsOnly, Category="Quest|Objective")
	FText ObjectiveDescription;
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Quest|Objective")
	TArray<UQuestCondition*> ObjectiveConditions;
	UPROPERTY(EditDefaultsOnly, Category="Quest|Objective")
	bool bIsOptional = false;
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Quest|Objective")
	TArray<UQuestCondition*> UnlockConditions;

protected:
	UPROPERTY(BlueprintReadOnly, Category="Quest|Objective")
	EQuestObjectiveStatus Status = EQuestObjectiveStatus::Locked;
};
