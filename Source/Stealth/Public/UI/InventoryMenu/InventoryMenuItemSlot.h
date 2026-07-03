#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Items/InventoryItem.h"
#include "PaperSprite.h"
#include "InventoryMenuItemSlot.generated.h"

class UButton;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemClicked, FInventoryItem, InventoryItem);

UCLASS()
class STEALTH_API UInventoryMenuItemSlot : public UUserWidget
{
	GENERATED_BODY()
	/*Events*/
public:
	UPROPERTY(BlueprintAssignable)
	FOnItemClicked OnItemClicked;
	/*Methods*/
public:
	UFUNCTION(BlueprintCallable)
	void Initialize(FInventoryItem NewInventoryItem);
	UFUNCTION(BlueprintCallable)
	void Clear();
	//Getters
	UFUNCTION(BlueprintPure)
	[[nodiscard]] const FInventoryItem& GetInventoryItem() const { return InventoryItem; }

protected:
	UFUNCTION()
	void OnItemButtonClicked();
	virtual void NativeConstruct() override;

	/*Properties*/
protected:
	UPROPERTY(BlueprintReadWrite)
	FInventoryItem InventoryItem;

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWidget> VisualsParent;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UImage> ItemImage;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemQuantityText;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> ItemButton;
};
