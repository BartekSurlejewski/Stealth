#pragma once

#include "CoreMinimal.h"
#include "Conditions/QuestCondition.h"
#include "QuestCondition_ItemOwned.generated.h"


UCLASS()
class STEALTH_API UQuestCondition_ItemOwned : public UQuestCondition
{
	GENERATED_BODY()

public:
	virtual bool IsConditionMet_Implementation(const UObject* WCO) const override;

protected:
	UPROPERTY(EditAnywhere, Category="Quest")
	FPrimaryAssetId RequiredItemID;
	UPROPERTY(EditAnywhere, Category="Quest")
	int32 MinCount = 1;
};
