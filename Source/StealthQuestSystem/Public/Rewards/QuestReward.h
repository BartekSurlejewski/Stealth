#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestReward.generated.h"


UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable)
class STEALTHQUESTSYSTEM_API UQuestReward : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Quest")
	bool GrantReward(const UObject* WorldContextObject) const;
	UFUNCTION(BlueprintNativeEvent, Category="Quest")
	FText GetRewardDescription() const;

	virtual bool GrantReward_Implementation(const UObject* WCO) const { return false; }
	virtual FText GetRewardDescription_Implementation() const { return FText::GetEmpty(); }
};
