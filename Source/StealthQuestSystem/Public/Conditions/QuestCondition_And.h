#pragma once

#include "CoreMinimal.h"
#include "Conditions/QuestCondition.h"
#include "QuestCondition_And.generated.h"

UCLASS()
class STEALTHQUESTSYSTEM_API UQuestCondition_And : public UQuestCondition
{
	GENERATED_BODY()

public:
	virtual bool IsConditionMet_Implementation(const UObject* WCO) const override;

protected:
	UPROPERTY(EditAnywhere, Instanced, Category="Quest")
	TArray<UQuestCondition*> SubConditions;
};
