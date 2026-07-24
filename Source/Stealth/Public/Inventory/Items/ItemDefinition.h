#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemDefinition.generated.h"

class UGameplayAbility;
class UItemEffect;
class UPaperSprite;

UCLASS(BlueprintType)
class STEALTH_API UItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

	/*Methods*/
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("ItemDefinition", GetFName());
	}

	/*Properties*/
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Name;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FText Description;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	bool bIsConsumable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TSoftObjectPtr<UPaperSprite> ItemImage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta=(MustImplement = "/Script/Stealth.Pickable"))
	TSoftClassPtr<AActor> PickupActorClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Behaviour")
	TSubclassOf<UItemEffect> EffectClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrantOnAdd;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrantOnEquip;
};
