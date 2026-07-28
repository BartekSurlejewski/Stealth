#include "Inventory/InventoryManagerSubsystem.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Messages/StealthMessages.h"
#include "Stealth/Stealth.h"

void UInventoryManagerSubsystem::RegisterPlayerInventory(UInventoryComponent* InventoryComponent)
{
	if (PlayerInventory.IsValid() && PlayerInventory.Get() != InventoryComponent)
	{
		UE_LOG(LogStealth, Warning, TEXT("[InventoryManager] Overriding already registered player inventory"));
	}
	PlayerInventory = InventoryComponent;
}

void UInventoryManagerSubsystem::UnregisterPlayerInventory(const UInventoryComponent* InventoryComponent)
{
	if (!PlayerInventory.IsValid() || PlayerInventory.Get() == InventoryComponent)
	{
		PlayerInventory.Reset();
	}
}

void UInventoryManagerSubsystem::RegisterPlayerStash(const FGuid& StashGuid, UInventoryComponent* InventoryComponent)
{
	if (!StashGuid.IsValid() || !InventoryComponent)
	{
		UE_LOG(LogStealth, Warning, TEXT("[InventoryManager] Invalid stash registration for Guid %s"), *StashGuid.ToString());
		return;
	}

	if (const UInventoryComponent* Existing = PlayerStashMap.FindRef(StashGuid))
	{
		if (Existing && Existing != InventoryComponent)
		{
			UE_LOG(LogStealth, Warning, TEXT("[InventoryManager] Stash GUID %s is already registered with a different component"),
			       *StashGuid.ToString());
		}
	}

	PlayerStashMap.Add(StashGuid, InventoryComponent);
}

void UInventoryManagerSubsystem::UnregisterPlayerStash(const FGuid& StashGuid, const UInventoryComponent* InventoryComponent)
{
	if (const UInventoryComponent* Existing = PlayerStashMap.FindRef(StashGuid))
	{
		if (!Existing || Existing == InventoryComponent)
		{
			PlayerStashMap.Remove(StashGuid);
		}
	}
}

void UInventoryManagerSubsystem::RegisterNpcInventory(const FGuid& NpcGuid, UInventoryComponent* InventoryComponent)
{
	if (!NpcGuid.IsValid() || !InventoryComponent)
	{
		UE_LOG(LogStealth, Warning, TEXT("[InventoryManager] Invalid NPC inventory registration for Guid %s"), *NpcGuid.ToString());
		return;
	}

	if (const UInventoryComponent* Found = NpcInventoryMap.FindRef(NpcGuid))
	{
		if (Found && Found != InventoryComponent)
		{
			UE_LOG(LogStealth, Warning, TEXT("[InventoryManager] NPC GUID %s is already registered with a different component"),
			       *NpcGuid.ToString());
		}
	}

	NpcInventoryMap.Add(NpcGuid, InventoryComponent);
}

void UInventoryManagerSubsystem::UnregisterNpcInventory(const FGuid& NpcGuid, const UInventoryComponent* InventoryComponent)
{
	if (const UInventoryComponent* Found = NpcInventoryMap.FindRef(NpcGuid))
	{
		if (!Found || Found == InventoryComponent)
		{
			NpcInventoryMap.Remove(NpcGuid);
		}
	}
}

UInventoryComponent* UInventoryManagerSubsystem::GetPlayerInventory() const
{
	return PlayerInventory.Get();
}

UInventoryComponent* UInventoryManagerSubsystem::GetPlayerStash(const FGuid& StashGuid) const
{
	if (UInventoryComponent* Found = PlayerStashMap.FindRef(StashGuid))
	{
		return Found;
	}

	return nullptr;
}

UInventoryComponent* UInventoryManagerSubsystem::GetNpcInventory(const FGuid& NpcGuid) const
{
	if (UInventoryComponent* Found = NpcInventoryMap.FindRef(NpcGuid))
	{
		return Found;
	}
	return nullptr;
}

TArray<UInventoryComponent*> UInventoryManagerSubsystem::GetAllNpcInventories()
{
	TArray<UInventoryComponent*> Result;
	for (auto It = NpcInventoryMap.CreateIterator(); It; ++It)
	{
		if (It.Value())
		{
			Result.Add(It.Value());
		}
		else
		{
			It.RemoveCurrent(); // Lazy cleanup of dead entries
		}
	}

	return Result;
}

bool UInventoryManagerSubsystem::TryRemoveItemFromPlayerInventory(UItemDefinition* ItemDefinition, const int32 QuantityToRemove, const bool bShouldDrop) const
{
	if (!PlayerInventory.IsValid())
	{
		UE_LOG(LogStealth, Error, TEXT("[InventoryManager] No Player Inventory registered"));
		return false;
	}

	int32 QuantityAfterRemove = 0;
	const bool bSucceeded = PlayerInventory->TryRemoveItem(ItemDefinition, QuantityAfterRemove, QuantityToRemove);

	if (bSucceeded)
	{
		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
		const FPlayerInventoryItemRemovedMessage Message(ItemDefinition, QuantityToRemove, QuantityAfterRemove, bShouldDrop);
		MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Inventory_ItemRemoved, Message);
	}

	return bSucceeded;
}

bool UInventoryManagerSubsystem::TryRemoveItemFromNpcInventory(const FGuid& NpcGuid, UItemDefinition* ItemDefinition, int32 QuantityToRemove, bool bShouldDrop) const
{
	UInventoryComponent* NpcInventory = GetNpcInventory(NpcGuid);
	if (!NpcInventory)
	{
		UE_LOG(LogStealth, Warning, TEXT("[InventoryManager] No NPC inventory registered for Guid %s"), *NpcGuid.ToString());
		return false;
	}

	int32 QuantityAfterRemove = 0;
	const bool bSucceeded = NpcInventory->TryRemoveItem(ItemDefinition, QuantityToRemove, QuantityAfterRemove);

	if (bSucceeded)
	{
		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
		const FNpcInventoryItemRemovedMessage Message(NpcGuid, ItemDefinition, QuantityToRemove, QuantityAfterRemove, bShouldDrop);
		MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Inventory_ItemRemoved, Message);
	}

	return bSucceeded;
}

bool UInventoryManagerSubsystem::TryAddItemToPlayerInventory(UItemDefinition* ItemDefinition, const int32 QuantityToAdd) const
{
	if (!PlayerInventory.IsValid())
	{
		UE_LOG(LogStealth, Error, TEXT("[InventoryManager] No Player Inventory registered"));
		return false;
	}

	int32 QuantityAfterAdd = 0;
	const bool bSucceeded = PlayerInventory->TryAddItem(ItemDefinition, QuantityAfterAdd, QuantityToAdd);

	if (bSucceeded)
	{
		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
		const FPlayerInventoryItemAddedMessage Message(ItemDefinition, QuantityToAdd, QuantityAfterAdd);
		MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Inventory_ItemAdded, Message);
	}

	return bSucceeded;
}

bool UInventoryManagerSubsystem::TryAddItemToNpcInventory(const FGuid& NpcGuid, UItemDefinition* ItemDefinition, const int32 QuantityToAdd) const
{
	UInventoryComponent* NpcInventory = GetNpcInventory(NpcGuid);
	if (!NpcInventory)
	{
		UE_LOG(LogStealth, Warning, TEXT("[InventoryManager] No NPC inventory registered for Guid %s"), *NpcGuid.ToString());
		return false;
	}

	int32 QuantityAfterAdd = 0;
	const bool bSucceeded = NpcInventory->TryAddItem(ItemDefinition, QuantityAfterAdd, QuantityToAdd);

	if (bSucceeded)
	{
		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
		const FNpcInventoryItemAddedMessage Message(NpcGuid, ItemDefinition, QuantityToAdd, QuantityAfterAdd);
		MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Inventory_ItemAdded, Message);
	}

	return bSucceeded;
}

bool UInventoryManagerSubsystem::TryMoveItemFromPlayerToNpc(const FGuid& NpcGuid, UItemDefinition* ItemDefinition, const int32 QuantityToMove, bool bSuccessAfterRemove) const
{
	if (!TryRemoveItemFromPlayerInventory(ItemDefinition, QuantityToMove, false))
	{
		return false;
	}

	bool bSucceeded = false;

	if (bSuccessAfterRemove)
	{
		TryAddItemToNpcInventory(NpcGuid, ItemDefinition, QuantityToMove);
		bSucceeded = true;
	}
	else
	{
		bSucceeded = TryAddItemToNpcInventory(NpcGuid, ItemDefinition, QuantityToMove);
	}

	if (bSucceeded)
	{
		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
		const FItemMovedFromPlayerToNpcMessage Message(NpcGuid, ItemDefinition, QuantityToMove);
		MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Inventory_ItemMoved, Message);
	}

	return bSucceeded;
}

bool UInventoryManagerSubsystem::TryMoveItemFromNpcToPlayer(const FGuid& NpcGuid, UItemDefinition* ItemDefinition, const int32 QuantityToMove) const
{
	if (!TryRemoveItemFromNpcInventory(NpcGuid, ItemDefinition, QuantityToMove, false))
	{
		return false;
	}

	const bool bSucceeded = TryAddItemToPlayerInventory(ItemDefinition, QuantityToMove);

	if (bSucceeded)
	{
		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
		const FItemMovedFromNpcToPlayerMessage Message(NpcGuid, ItemDefinition, QuantityToMove);
		MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Inventory_ItemMoved, Message);
	}

	return bSucceeded;
}
