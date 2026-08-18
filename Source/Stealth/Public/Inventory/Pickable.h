#pragma once

#include "CoreMinimal.h"
#include "Items/ItemDefinition.h"
#include "UObject/Interface.h"
#include "Pickable.generated.h"

class AStealthPlayerCharacter;

UINTERFACE(MinimalAPI, BlueprintType)
class UPickable : public UInterface
{
	GENERATED_BODY()
};


class STEALTH_API IPickable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void Initialize(UItemDefinition* NewItemDefinition, const int NewQuantity, const FText& InteractPromptText = FText::GetEmpty());
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Pickup")
	void OnPickup();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Pickup")
	void OnDrop();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Pickup")
	UItemDefinition* GetInventoryItem() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Pickup")
	int GetQuantity() const;
};
