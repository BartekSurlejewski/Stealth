#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "InventoryItem.generated.h"

class UItemDefinition;

USTRUCT(BlueprintType)
struct STEALTH_API FInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<UItemDefinition> ItemDefinition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Quantity;

	bool operator==(const FInventoryItem& Other) const
	{
		return ItemDefinition == Other.ItemDefinition;
	}
};
