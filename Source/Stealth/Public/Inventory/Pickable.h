#pragma once

#include "CoreMinimal.h"
#include "Item.h"
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
};
