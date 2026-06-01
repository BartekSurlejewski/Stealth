#include "Inventory/InventoryComponent.h"

#include "Inventory/Items/InventoryItem.h"
#include "Inventory/Items/ItemDefinition.h"
#include "Inventory/Items/ItemEffect.h"


UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UInventoryComponent::TryAddItem(UItemDefinition* ItemDefinition, int32 QuantityToAdd)
{
	if (!ItemDefinition || QuantityToAdd <= 0)
	{
		return false;
	}

	// Try stacking into an existing slot first
	if (FInventoryItem* ExistingSlot = FindItemSlot(ItemDefinition))
	{
		ExistingSlot->Quantity += QuantityToAdd;
		OnInventoryChanged.Broadcast();
		OnItemAdded.Broadcast(*ExistingSlot, QuantityToAdd, ExistingSlot->Quantity);
		return true;
	}

	// Need a new slot
	if (IsFull())
	{
		return false;
	}

	FInventoryItem NewItem;
	NewItem.ItemDefinition = ItemDefinition;
	NewItem.Quantity = QuantityToAdd;

	Items.Add(NewItem);
	OnInventoryChanged.Broadcast();
	OnItemAdded.Broadcast(NewItem, QuantityToAdd, NewItem.Quantity);
	return true;
}

bool UInventoryComponent::TryRemoveItem(const UItemDefinition* ItemDefinition, const int32 QuantityToRemove, bool bShouldDrop)
{
	if (!ItemDefinition || QuantityToRemove <= 0)
	{
		return false;
	}

	FInventoryItem* Slot = FindItemSlot(ItemDefinition);
	if (!Slot)
	{
		return false;
	}
	if (Slot->Quantity < QuantityToRemove) return false;

	// Cache for delegate before we modify/remove
	FInventoryItem RemovedSnapshot = *Slot;
	RemovedSnapshot.Quantity = QuantityToRemove;

	Slot->Quantity = FMath::Max(0, Slot->Quantity - QuantityToRemove);

	if (Slot->Quantity <= 0)
	{
		// Unequip if this was the equipped item
		if (EquippedItemDefinition == ItemDefinition)
		{
			TryUnequipItem(ItemDefinition);
		}

		Items.RemoveSingle(*Slot);
	}

	OnInventoryChanged.Broadcast();
	OnItemRemoved.Broadcast(RemovedSnapshot, QuantityToRemove, Slot->Quantity, bShouldDrop);
	return true;
}

bool UInventoryComponent::TryUseItem(const UItemDefinition* ItemDefinition)
{
	if (!ItemDefinition || !HasItem(ItemDefinition)) return false;

	UItemEffect* Effect = CreateEffect(ItemDefinition);
	if (Effect)
	{
		Effect->OnUse(GetOwner());
	}

	// Consumables remove themselves after use
	// (drive this from the definition if you want non-consumables to be usable too)
	if (ItemDefinition->bIsConsumable)
	{
		TryRemoveItem(ItemDefinition, 1);
	}

	return true;
}

bool UInventoryComponent::TryEquipItem(UItemDefinition* ItemDefinition)
{
	if (!ItemDefinition || !HasItem(ItemDefinition)) return false;

	// Unequip current item first
	if (EquippedItemDefinition)
	{
		TryUnequipItem(EquippedItemDefinition);
	}

	UItemEffect* Effect = CreateEffect(ItemDefinition);
	if (Effect)
	{
		Effect->OnEquip(GetOwner());
		ActiveEquippedEffect = Effect;
		EquippedItemDefinition = ItemDefinition;
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::TryUnequipItem(const UItemDefinition* ItemDefinition)
{
	if (!ItemDefinition || EquippedItemDefinition != ItemDefinition) return false;

	if (ActiveEquippedEffect)
	{
		ActiveEquippedEffect->OnUnequip(GetOwner());
		ActiveEquippedEffect = nullptr;
	}

	EquippedItemDefinition = nullptr;
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::HasItem(const UItemDefinition* ItemDefinition, int32 Quantity) const
{
	return GetItemQuantity(ItemDefinition) >= Quantity;
}

int32 UInventoryComponent::GetItemQuantity(const UItemDefinition* ItemDefinition) const
{
	for (const FInventoryItem& Slot : Items)
	{
		if (Slot.ItemDefinition == ItemDefinition)
		{
			return Slot.Quantity;
		}
	}
	return 0;
}

bool UInventoryComponent::IsFull() const
{
	return MaxSlots > 0 && Items.Num() >= MaxSlots;
}

FInventoryItem* UInventoryComponent::FindItemSlot(const UItemDefinition* ItemDefinition)
{
	return Items.FindByPredicate([ItemDefinition](const FInventoryItem& Slot)
	{
		return Slot.ItemDefinition == ItemDefinition;
	});
}

UItemEffect* UInventoryComponent::CreateEffect(const UItemDefinition* ItemDefinition) const
{
	if (!ItemDefinition || !ItemDefinition->EffectClass)
	{
		return nullptr;
	}

	return NewObject<UItemEffect>(GetOwner(), ItemDefinition->EffectClass);
}
