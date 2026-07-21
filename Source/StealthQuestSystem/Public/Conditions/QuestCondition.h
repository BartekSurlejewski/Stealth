#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestCondition.generated.h"

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable)
class STEALTHQUESTSYSTEM_API UQuestCondition : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Quest")
	bool IsConditionMet(const UObject* WorldContextObject) const;
	// Short description for UI/debug, like "Player lvl >= 5"
	UFUNCTION(BlueprintNativeEvent, Category="Quest")
	FText GetDescription() const;

	virtual bool IsConditionMet_Implementation(const UObject* WCO) const { return false; }
	virtual FText GetDescription_Implementation() const { return FText::GetEmpty(); }
};
