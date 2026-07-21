#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestObjectiveDefinition.generated.h"


class UQuestCondition;

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, Blueprintable)
class STEALTHQUESTSYSTEM_API UQuestObjectiveDefinition : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Quest")
	FName ObjectiveID;
	UPROPERTY(EditDefaultsOnly, Category="Quest")
	FText ObjectiveDescription;
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Quest")
	TArray<UQuestCondition*> EventFilterConditions;
	UPROPERTY(EditDefaultsOnly, Category="Quest")
	bool bIsOptional = false;
};
