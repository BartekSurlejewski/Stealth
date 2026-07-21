#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "QuestCondition.h"
#include "QuestCondition_GameplayTagPresent.generated.h"


class UAbilitySystemComponent;

UCLASS()
class STEALTHQUESTSYSTEM_API UQuestCondition_GameplayTagPresent : public UQuestCondition
{
	GENERATED_BODY()

public:
	virtual bool IsConditionMet_Implementation(const UObject* WCO) const override;

protected:
	static const UAbilitySystemComponent* GetPlayerASC(const UObject* WorldContextObject);

protected:
	UPROPERTY(EditAnywhere, Category="Quest")
	FGameplayTag RequiredTag;
	UPROPERTY(EditAnywhere, Category="Quest")
	bool bExactMatch = false;
};
