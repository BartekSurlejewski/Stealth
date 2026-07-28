#include "Quests/Conditions/QuestCondition_ItemOwned.h"

#include "GameFramework/GameplayMessageSubsystem.h"
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

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	PlayerInventoryItemAddedHandle = MsgSubsystem.RegisterListener<FPlayerInventoryItemAddedMessage>(StealthMessageChannels::TAG_Message_Inventory_ItemAdded, this,
	                                                                                                 &UQuestCondition_ItemOwned::PlayerInventory_OnItemAdded);
	PlayerInventoryItemRemovedHandle = MsgSubsystem.RegisterListener<FPlayerInventoryItemRemovedMessage>(StealthMessageChannels::TAG_Message_Inventory_ItemRemoved, this,
	                                                                                                     &UQuestCondition_ItemOwned::PlayerInventory_OnItemRemoved);
}

void UQuestCondition_ItemOwned::Deactivate_Implementation()
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(PlayerInventoryItemAddedHandle);
	MsgSubsystem.UnregisterListener(PlayerInventoryItemRemovedHandle);

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

void UQuestCondition_ItemOwned::PlayerInventory_OnItemAdded(FGameplayTag Channel, const FPlayerInventoryItemAddedMessage& Message)
{
	if (Message.AddedItem->GetPrimaryAssetId() == RequiredItemID && Message.QuantityInInventory >= MinCount)
	{
		OnQuestConditionStatusChanged.Broadcast(this, true);
	}
}

void UQuestCondition_ItemOwned::PlayerInventory_OnItemRemoved(FGameplayTag Channel, const FPlayerInventoryItemRemovedMessage& Message)
{
	if (Message.RemovedItem->GetPrimaryAssetId() == RequiredItemID && Message.QuantityInInventory < MinCount)
	{
		OnQuestConditionStatusChanged.Broadcast(this, false);
	}
}
