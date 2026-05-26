#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

class UItemEffect;
class UPaperSprite;

UCLASS(BlueprintType)
class STEALTH_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Name;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bIsStackable = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bIsConsumable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 1))
	int32 MaxStackSize = 99;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UPaperSprite> ItemImage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta=(MustImplement = "/Script/Stealth.Pickable"))
	TSubclassOf<AActor> ItemActorClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Behaviour")
	TSubclassOf<UItemEffect> EffectClass;
};
