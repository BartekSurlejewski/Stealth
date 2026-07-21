#pragma once

#include "CoreMinimal.h"
#include "QuestCondition.h"
#include "QuestManagerSubsystem.h"
#include "QuestCondition_QuestCompleted.generated.h"


UCLASS()
class STEALTHQUESTSYSTEM_API UQuestCondition_QuestCompleted : public UQuestCondition
{
	GENERATED_BODY()

public:
	virtual bool IsConditionMet_Implementation(const UObject* WCO) const override
	{
		auto* QuestManager = UQuestManagerSubsystem::Get(WCO);
		return QuestManager->IsQuestCompleted(RequiredQuestID);
	}

protected:
	UPROPERTY(EditAnywhere, Category="Quest")
	FPrimaryAssetId RequiredQuestID;
};
