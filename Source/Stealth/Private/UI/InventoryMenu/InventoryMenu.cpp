#include "UI/InventoryMenu/InventoryMenu.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/Items/ItemDefinition.h"
#include "UI/InventoryMenu/InventoryMenuItemSlot.h"

void UInventoryMenu::NativeConstruct()
{
	Super::NativeConstruct();
	if (!ItemsContainer)
	{
		return;
	}

	InventoryItemsSlots = TArray<UInventoryMenuItemSlot*>();
	for (UWidget* Child : ItemsContainer->GetAllChildren())
	{
		if (auto ItemWidget = Cast<UInventoryMenuItemSlot>(Child))
		{
			InventoryItemsSlots.AddUnique(ItemWidget);
			ItemWidget->OnItemClicked.AddDynamic(this, &UInventoryMenu::ItemSlot_OnItemClicked);
		}
	}

	if (DropSelectedItemButton)
	{
		DropSelectedItemButton->OnClicked.AddDynamic(this, &UInventoryMenu::DropButton_OnClicked);
	}
}

void UInventoryMenu::Initialize(UInventoryComponent* Inventory)
{
	this->CurrentInventory = Inventory;
	const auto InventoryItems = Inventory->GetItems();

	//TODO: Rework, now assumes that theres always more ItemSlots than items in inventory
	for (int i = 0; i < InventoryItemsSlots.Num(); i++)
	{
		if (i < InventoryItems.Num())
		{
			InventoryItemsSlots[i]->Initialize(InventoryItems[i]);
		}
		else
		{
			InventoryItemsSlots[i]->Clear();
		}
	}
}

void UInventoryMenu::ShowItemInfo(const FInventoryItem InventoryItem)
{
	SelectedItemImage->SetBrushFromAtlasInterface(InventoryItem.ItemDefinition->ItemImage);
	SelectedItemName->SetText(InventoryItem.ItemDefinition->Name);
	SelectedItemDescription->SetText(InventoryItem.ItemDefinition->Description);
}

void UInventoryMenu::ItemSlot_OnItemClicked(FInventoryItem InventoryItem)
{
	if (!InventoryItem.ItemDefinition)
	{
		return;
	}

	SelectedItem = InventoryItem;
	ShowItemInfo(SelectedItem);
}

void UInventoryMenu::DropButton_OnClicked()
{
	if (!CurrentInventory || !SelectedItem.ItemDefinition)
	{
		return;
	}

	//TODO: Rework
	CurrentInventory->RemoveItem(SelectedItem.ItemDefinition, SelectedItem.Amount);
	auto MatchingSlot = InventoryItemsSlots.FindByPredicate([&](const UInventoryMenuItemSlot* ItemSlot)
	{
		return ItemSlot->GetInventoryItem().ItemDefinition == SelectedItem.ItemDefinition && ItemSlot->GetInventoryItem().Amount == SelectedItem.Amount;
	});

	if (MatchingSlot)
	{
		UInventoryMenuItemSlot* InventorySlot = *MatchingSlot;
		InventorySlot->Clear();
	}
}
