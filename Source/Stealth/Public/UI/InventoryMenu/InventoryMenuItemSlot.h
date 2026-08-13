#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Items/InventoryItem.h"
#include "UI/Core/StealthUserWidget.h"
#include "InventoryMenuItemSlot.generated.h"

class UButton;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemClicked, const FInventoryItem&, InventoryItem);

UCLASS()
class STEALTH_API UInventoryMenuItemSlot : public UStealthUserWidget
{
	GENERATED_BODY()
	/*Events*/
public:
	UPROPERTY(BlueprintAssignable)
	FOnItemClicked OnItemClicked;
	/*Methods*/
public:
	UFUNCTION(BlueprintCallable)
	void SetItem(FInventoryItem NewInventoryItem);

	//Getters
	UFUNCTION(BlueprintPure)
	[[nodiscard]] const FInventoryItem& GetInventoryItem() const { return InventoryItem; }

protected:
	virtual void NativeConstruct() override;
virtual void OnHide_Implementation() override;	
	UFUNCTION()
	void OnItemButtonClicked();

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
