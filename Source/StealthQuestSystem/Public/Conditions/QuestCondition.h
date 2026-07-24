#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "QuestCondition.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestConditionStatusChanged, UQuestCondition*, QuestCondition, bool, bIsConditionMet);

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable)
class STEALTHQUESTSYSTEM_API UQuestCondition : public UObject
{
	GENERATED_BODY()

	/*Events*/
public:
	UPROPERTY(BlueprintAssignable, Category="Quest|Condition")
	FOnQuestConditionStatusChanged OnQuestConditionStatusChanged;

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Quest")
	void Activate();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Quest")
	void Deactivate();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Quest")
	bool IsConditionMet(const UObject* WorldContextObject) const;
	// Short description for UI/debug, like "Player lvl >= 5"
	UFUNCTION(BlueprintNativeEvent, Category="Quest")
	FText GetDescription() const;

	UFUNCTION(BlueprintNativeEvent, Category="Quest")
	void ForceComplete(const UObject* WorldContextObject) const;

	virtual void Activate_Implementation() {};
	virtual void Deactivate_Implementation() {};
	virtual bool IsConditionMet_Implementation(const UObject* WCO) const { return false; }
	virtual FText GetDescription_Implementation() const { return FText::GetEmpty(); }

	virtual void ForceComplete_Implementation(const UObject* WorldContextObject) const
	{
		OnQuestConditionStatusChanged.Broadcast(const_cast<UQuestCondition*>(this), true);
	}
};
