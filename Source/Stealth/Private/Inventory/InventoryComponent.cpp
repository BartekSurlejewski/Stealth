#include "Inventory/InventoryComponent.h"

#include "Inventory/Item.h"


UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UInventoryComponent::AddItem(UItemDefinition* ItemDefinition, int32 AmountToAdd)
{
	if (!ItemDefinition || AmountToAdd <= 0)
	{
		return false;
	}

	// Try stacking into an existing slot first
	if (ItemDefinition->bIsStackable)
	{
		if (FInventoryItem* ExistingSlot = FindItemSlot(ItemDefinition))
		{
			const int32 SpaceLeft = ItemDefinition->MaxStackSize - ExistingSlot->Amount;
			const int32 AmountToAddClamped = FMath::Min(AmountToAdd, SpaceLeft);

			if (AmountToAddClamped <= 0) return false; // stack full

			ExistingSlot->Amount += AmountToAddClamped;
			OnInventoryChanged.Broadcast();
			OnItemAdded.Broadcast(*ExistingSlot);
			return true;
		}
	}

	// Need a new slot
	if (IsFull())
	{
		return false;
	}

	FInventoryItem NewItem;
	NewItem.ItemDefinition = ItemDefinition;
	NewItem.Amount = FMath::Min(AmountToAdd, ItemDefinition->bIsStackable ? ItemDefinition->MaxStackSize : 1);

	Items.Add(NewItem);
	OnInventoryChanged.Broadcast();
	OnItemAdded.Broadcast(NewItem);
	return true;
}

bool UInventoryComponent::RemoveItem(const UItemDefinition* ItemDefinition, int32 AmountToRemove)
{
	if (!ItemDefinition || AmountToRemove <= 0) return false;

	FInventoryItem* Slot = FindItemSlot(ItemDefinition);
	if (!Slot) return false;
	if (Slot->Amount < AmountToRemove) return false;

	// Cache for delegate before we modify/remove
	FInventoryItem RemovedSnapshot = *Slot;
	RemovedSnapshot.Amount = AmountToRemove;

	Slot->Amount -= AmountToRemove;

	if (Slot->Amount <= 0)
	{
		// Unequip if this was the equipped item
		if (EquippedItemDefinition == ItemDefinition)
		{
			UnequipItem(ItemDefinition);
		}
		
		Items.RemoveSingle(*Slot);
	}

	OnInventoryChanged.Broadcast();
	OnItemRemoved.Broadcast(RemovedSnapshot);
	return true;
}

bool UInventoryComponent::UseItem(const UItemDefinition* ItemDefinition)
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
		RemoveItem(ItemDefinition, 1);
	}

	return true;
}

bool UInventoryComponent::EquipItem(UItemDefinition* ItemDefinition)
{
	if (!ItemDefinition || !HasItem(ItemDefinition)) return false;

	// Unequip current item first
	if (EquippedItemDefinition)
	{
		UnequipItem(EquippedItemDefinition);
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

bool UInventoryComponent::UnequipItem(const UItemDefinition* ItemDefinition)
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

bool UInventoryComponent::HasItem(const UItemDefinition* ItemDefinition, int32 Amount) const
{
	return GetItemAmount(ItemDefinition) >= Amount;
}

int32 UInventoryComponent::GetItemAmount(const UItemDefinition* ItemDefinition) const
{
	for (const FInventoryItem& Slot : Items)
	{
		if (Slot.ItemDefinition == ItemDefinition)
		{
			return Slot.Amount;
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
