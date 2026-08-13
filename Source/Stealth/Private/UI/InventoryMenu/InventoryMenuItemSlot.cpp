#include "UI/InventoryMenu/InventoryMenuItemSlot.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Inventory/Items/ItemDefinition.h"
#include "PaperSprite.h"

void UInventoryMenuItemSlot::SetItem(FInventoryItem NewInventoryItem)
{
	if (!VisualsParent || !ItemImage || !ItemQuantityText)
	{
		return;
	}

	InventoryItem = NewInventoryItem;
	VisualsParent->SetVisibility(ESlateVisibility::Visible);
	ItemImage->SetBrushFromAtlasInterface(InventoryItem.ItemDefinition->ItemImage.LoadSynchronous());
	ItemQuantityText->SetText(FText::AsNumber(InventoryItem.Quantity));
}

void UInventoryMenuItemSlot::Hide()
{
	if (!VisualsParent)
	{
		return;
	}

	InventoryItem = FInventoryItem();

	Super::Hide();
}

void UInventoryMenuItemSlot::OnItemButtonClicked()
{
	if (VisualsParent->GetVisibility() != ESlateVisibility::Visible)
	{
		return;
	}

	OnItemClicked.Broadcast(InventoryItem);
}

void UInventoryMenuItemSlot::NativeConstruct()
{
	Super::NativeConstruct();
	ItemButton->OnClicked.AddUniqueDynamic(this, &UInventoryMenuItemSlot::OnItemButtonClicked);
}
