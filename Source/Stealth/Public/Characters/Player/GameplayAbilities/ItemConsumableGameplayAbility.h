#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "ItemConsumableGameplayAbility.generated.h"

class UItemDefinition;
class UInventoryComponent;

UCLASS()
class STEALTH_API UItemConsumableGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr,
	                                const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	                             const FGameplayEventData* TriggerEventData) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UItemDefinition> NeededItem;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	int NeededItemQuantity;

	UPROPERTY()
	mutable TObjectPtr<UInventoryComponent> CachedInventoryComponent;
};
