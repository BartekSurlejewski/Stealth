#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Items/InventoryItem.h"
#include "InventoryMenu.generated.h"


class UInventoryMenuItemSlot;
class UButton;
class UImage;
class UWrapBox;
class UInventoryComponent;
class UTextBlock;

UCLASS(Abstract, Blueprintable)
class STEALTH_API UInventoryMenu : public UUserWidget
{
	GENERATED_BODY()
	/*Methods*/
protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable)
	void Initialize(UInventoryComponent* Inventory);

protected:
	UFUNCTION(BlueprintCallable)
	void ShowItemInfo(FInventoryItem InventoryItem);
	UFUNCTION()
	void ItemSlot_OnItemClicked(FInventoryItem InventoryItem);
	UFUNCTION()
	void DropButton_OnClicked();

	/*Properties*/
protected:
	UPROPERTY(BlueprintReadOnly)
	TArray<UInventoryMenuItemSlot*> InventoryItemsSlots;
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UInventoryComponent> CurrentInventory;
	UPROPERTY(BlueprintReadWrite)
	FInventoryItem SelectedItem;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UWrapBox> ItemsContainer;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> SelectedItemImage;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> SelectedItemName;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> SelectedItemDescription;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> SelectedItemLegality;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> DropSelectedItemButton;
};
