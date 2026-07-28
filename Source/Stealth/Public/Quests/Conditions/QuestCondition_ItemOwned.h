#pragma once

#include "CoreMinimal.h"
#include "Conditions/QuestCondition.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/StealthMessages.h"
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
	void PlayerInventory_OnItemAdded(FGameplayTag Channel, const FPlayerInventoryItemAddedMessage& Message);
	UFUNCTION()
	void PlayerInventory_OnItemRemoved(FGameplayTag Channel, const FPlayerInventoryItemRemovedMessage& Message);

protected:
	UPROPERTY(EditAnywhere, Category="Quest", meta=(AllowedTypes="ItemDefinition"))
	FPrimaryAssetId RequiredItemID;
	UPROPERTY(EditAnywhere, Category="Quest")
	int32 MinCount = 1;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> PlayerInventoryComponent;

private:
	FGameplayMessageListenerHandle PlayerInventoryItemAddedHandle;
	FGameplayMessageListenerHandle PlayerInventoryItemRemovedHandle;
};
