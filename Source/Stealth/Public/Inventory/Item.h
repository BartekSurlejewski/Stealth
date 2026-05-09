#pragma once

#include "CoreMinimal.h"
#include "UObject/Class.h"
#include "Item.generated.h"


// Base class for custom item logic
UCLASS(Abstract, BlueprintType, Blueprintable)
class STEALTH_API UItemEffect : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void OnUse(AActor* InstigatorActor);

	UFUNCTION(BlueprintNativeEvent)
	void OnEquip(AActor* InstigatorActor);

	UFUNCTION(BlueprintNativeEvent)
	void OnUnequip(AActor* InstigatorActor);

	virtual void OnUse_Implementation(AActor* InstigatorActor) {}
	virtual void OnEquip_Implementation(AActor* InstigatorActor) {}
	virtual void OnUnequip_Implementation(AActor* InstigatorActor) {}
};

UCLASS(BlueprintType)
class STEALTH_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Name;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bIsStackable = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bIsConsumable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (ClampMin = 1))
	int32 MaxStackSize = 99;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSoftObjectPtr<UTexture2D> ItemImage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSubclassOf<AActor> ItemActorClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Behaviour")
	TSubclassOf<UItemEffect> EffectClass;
};

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSoftObjectPtr<UItemDefinition> ItemDefinition;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 Amount;

	bool operator==(const FInventoryItem& Other) const
	{
		return ItemDefinition == Other.ItemDefinition;
	}
};
