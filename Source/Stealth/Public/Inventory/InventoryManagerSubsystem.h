#pragma once

#include "CoreMinimal.h"
#include "Core/StealthWorldSubsystem.h"
#include "InventoryManagerSubsystem.generated.h"

class UNpcRegistrySubsystem;
class UItemDefinition;
class UInventoryComponent;

UCLASS()
class STEALTH_API UInventoryManagerSubsystem : public UStealthWorldSubsystem
{
	GENERATED_BODY()
	/*Methods*/
public:
	static UInventoryManagerSubsystem* Get(const UObject* WorldContextObject)
	{
		return UStealthWorldSubsystem::Get<UInventoryManagerSubsystem>(WorldContextObject);
	}

	// Entries management
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	void RegisterPlayerInventory(UInventoryComponent* InventoryComponent);
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	void UnregisterPlayerInventory(const UInventoryComponent* InventoryComponent);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	void RegisterPlayerStash(const FGuid& StashGuid, UInventoryComponent* InventoryComponent);
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	void UnregisterPlayerStash(const FGuid& StashGuid, const UInventoryComponent* InventoryComponent);

	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	UInventoryComponent* GetPlayerInventory() const;
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	UInventoryComponent* GetPlayerStash(const FGuid& StashGuid) const;
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	UInventoryComponent* GetNpcInventory(const FGuid& NpcGuid) const;

	// Inventories Operations
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	bool TryRemoveItemFromPlayerInventory(UItemDefinition* ItemDefinition, int32 QuantityToRemove = 1, bool bShouldDrop = false) const;
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	bool TryRemoveItemFromNpcInventory(const FGuid& NpcGuid, UItemDefinition* ItemDefinition, int32 QuantityToRemove, bool bShouldDrop = false) const;
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	bool TryAddItemToPlayerInventory(UItemDefinition* ItemDefinition, const int32 QuantityToAdd) const;
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	bool TryAddItemToNpcInventory(const FGuid& NpcGuid, UItemDefinition* ItemDefinition, const int32 QuantityToAdd) const;
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	bool TryMoveItemFromPlayerToNpc(const FGuid& NpcGuid, UItemDefinition* ItemDefinition, const int32 QuantityToMove, bool bSuccessAfterRemove) const;
	UFUNCTION(BlueprintCallable, Category = "Inventory Manager")
	bool TryMoveItemFromNpcToPlayer(const FGuid& NpcGuid, UItemDefinition* ItemDefinition, const int32 QuantityToMove) const;

	/*Properties*/
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Manager", meta = (AllowPrivateAccess = "true"))
	TMap<FGuid, UInventoryComponent*> PlayerStashMap;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory Manager", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<UInventoryComponent> PlayerInventory;
};
