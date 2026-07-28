#pragma once

#include "CoreMinimal.h"
#include "Objectives/QuestObjective.h"
#include "QuestObjective_DeliverItem.generated.h"

UCLASS()
class STEALTH_API UQuestObjective_DeliverItem : public UQuestObjective
{
	GENERATED_BODY()

public:
	virtual void ActivateObjective_Implementation() override;
	virtual void DeactivateObjective_Implementation() override;
};
