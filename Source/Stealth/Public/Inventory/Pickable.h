#pragma once

#include "CoreMinimal.h"
#include "Items/ItemDefinition.h"
#include "UObject/Interface.h"
#include "Pickable.generated.h"


UINTERFACE(MinimalAPI, BlueprintType)
class UPickable : public UInterface
{
	GENERATED_BODY()
};


class STEALTH_API IPickable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Pickup")
	UItemDefinition* GetInventoryItem() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Pickup")
	int GetAmount() const;
};
