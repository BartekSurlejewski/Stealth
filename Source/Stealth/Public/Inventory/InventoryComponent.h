#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/InventoryItem.h"
#include "InventoryComponent.generated.h"

class UItemEffect;
class UItemDefinition;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	/*Methods*/
public:
	UInventoryComponent();
	virtual void BeginPlay() override;

	// --- Core API ---
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryAddItem(UItemDefinition* ItemDefinition, int32& QuantityAfterAdd, int32 QuantityToAdd = 1);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryRemoveItem(const UItemDefinition* ItemDefinition, int32& QuantityAfterRemove, int32 QuantityToRemove = 1);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryUseItem(const UItemDefinition* ItemDefinition);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryEquipItem(UItemDefinition* ItemDefinition);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryUnequipItem(const UItemDefinition* ItemDefinition);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool HasItem(const UItemDefinition* ItemDefinition, int32 Quantity = 1) const;
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemQuantity(const UItemDefinition* ItemDefinition) const;
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemQuantityByID(FPrimaryAssetId ItemDefID) const;
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsFull() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<FInventoryItem> GetItems() const
	{
		return Items;
	}

private:
	// Returns pointer to existing slot or nullptr
	FInventoryItem* FindItemSlot(const UItemDefinition* ItemDefinition);
	// Instantiates and returns an effect object from a definition
	UItemEffect* CreateEffect(const UItemDefinition* ItemDefinition) const;

	/*Properties*/
private:
	// TODO: Rework to use map for O(1) operations
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TArray<FInventoryItem> Items;
	// UPROPERTY()
	TMap<TObjectPtr<UItemDefinition>, TArray<int32>> ItemSlotsMap;
	// UPROPERTY()
	TArray<int32> FreeSlotIndices;

	// Max number of slots (0 = unlimited)
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxSlots = 20;

	// Currently equipped item effect (only one at a time)
	UPROPERTY()
	TObjectPtr<UItemEffect> ActiveEquippedEffect;
	UPROPERTY()
	TObjectPtr<UItemDefinition> EquippedItemDefinition;
};
