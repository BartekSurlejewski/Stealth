#pragma once

#include "CoreMinimal.h"
#include "Conditions/QuestCondition.h"
#include "QuestCondition_ItemOwned.generated.h"


struct FInventoryItem;
class UInventoryComponent;

UCLASS()
class STEALTH_API UQuestCondition_ItemOwned : public UQuestCondition
{
	GENERATED_BODY()

public:
	virtual void Activate_Implementation() override;
	virtual void Deactivate_Implementation() override;
	virtual bool IsConditionMet_Implementation(const UObject* WCO) const override;

private:
	UFUNCTION()
	void PlayerInventory_OnItemAdded(FInventoryItem& Item, int32 RemovedQuantity, int32 QuantityInInventory);
	UFUNCTION()
	void PlayerInventory_OnItemRemoved(FInventoryItem& Item, int32 RemovedQuantity, int32 QuantityInInventory, bool bShouldDrop);

protected:
	UPROPERTY(EditAnywhere, Category="Quest", meta=(AllowedTypes="ItemDefinition"))
	FPrimaryAssetId RequiredItemID;
	UPROPERTY(EditAnywhere, Category="Quest")
	int32 MinCount = 1;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> PlayerInventoryComponent;
};
