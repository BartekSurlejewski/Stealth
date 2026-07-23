#pragma once

#include "CoreMinimal.h"
#include "QuestCondition.h"
#include "QuestCondition_ObjectiveCompleted.generated.h"


UCLASS()
class STEALTHQUESTSYSTEM_API UQuestCondition_ObjectiveCompleted : public UQuestCondition
{
	GENERATED_BODY()
	/*Methods*/
public:
	virtual bool IsConditionMet_Implementation(const UObject* WorldContextObject) const override;
	virtual FText GetDescription_Implementation() const override;

	/*Properties*/
protected:
	UPROPERTY(EditAnywhere, Category="Quest")
	FPrimaryAssetId QuestID;
	UPROPERTY(EditAnywhere, Category="Quest")
	FName ObjectiveID;
};
