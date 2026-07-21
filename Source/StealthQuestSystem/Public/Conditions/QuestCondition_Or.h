#pragma once

#include "CoreMinimal.h"
#include "Conditions/QuestCondition.h"
#include "QuestCondition_Or.generated.h"


UCLASS()
class STEALTHQUESTSYSTEM_API UQuestCondition_Or : public UQuestCondition
{
	GENERATED_BODY()

public:
	virtual bool IsConditionMet_Implementation(const UObject* WCO) const override;

protected:
	UPROPERTY(EditAnywhere, Instanced, Category="Quest")
	TArray<UQuestCondition*> SubConditions;
};
