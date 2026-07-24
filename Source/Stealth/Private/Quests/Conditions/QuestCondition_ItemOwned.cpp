#include "Quests/Conditions/QuestCondition_ItemOwned.h"

#include "GameFramework/PlayerState.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/Items/ItemDefinition.h"
#include "Kismet/GameplayStatics.h"

void UQuestCondition_ItemOwned::Activate_Implementation()
{
	Super::Activate_Implementation();

	APlayerState* PlayerState = UGameplayStatics::GetPlayerState(this, 0);
	if (!PlayerState)
	{
		return;
	}

	PlayerInventoryComponent = PlayerState->GetComponentByClass<UInventoryComponent>();
	if (!PlayerInventoryComponent)
	{
		return;
	}

	PlayerInventoryComponent->OnItemAdded.AddDynamic(this, &UQuestCondition_ItemOwned::PlayerInventory_OnItemAdded);
	PlayerInventoryComponent->OnItemRemoved.AddDynamic(this, &UQuestCondition_ItemOwned::PlayerInventory_OnItemRemoved);
}

void UQuestCondition_ItemOwned::Deactivate_Implementation()
{
	if (PlayerInventoryComponent)
	{
		PlayerInventoryComponent->OnItemAdded.RemoveAll(this);
		PlayerInventoryComponent->OnItemRemoved.RemoveAll(this);
	}
	PlayerInventoryComponent = nullptr;

	Super::Deactivate_Implementation();
}

bool UQuestCondition_ItemOwned::IsConditionMet_Implementation(const UObject* WCO) const
{
	if (!PlayerInventoryComponent)
	{
		return false;
	}

	return PlayerInventoryComponent->GetItemQuantityByID(RequiredItemID) >= MinCount;
}

void UQuestCondition_ItemOwned::PlayerInventory_OnItemAdded(FInventoryItem& Item, int32 RemovedQuantity, int32 QuantityInInventory)
{
	if (Item.ItemDefinition->GetPrimaryAssetId() == RequiredItemID && QuantityInInventory >= MinCount)
	{
		OnQuestConditionStatusChanged.Broadcast(this, true);
	}
}

void UQuestCondition_ItemOwned::PlayerInventory_OnItemRemoved(FInventoryItem& Item, int32 RemovedQuantity, int32 QuantityInInventory, bool bShouldDrop)
{
	if (Item.ItemDefinition->GetPrimaryAssetId() == RequiredItemID && QuantityInInventory < MinCount)
	{
		OnQuestConditionStatusChanged.Broadcast(this, false);
	}
}
