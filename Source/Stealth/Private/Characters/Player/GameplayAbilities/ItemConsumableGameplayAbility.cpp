#include "Characters/Player/GameplayAbilities/ItemConsumableGameplayAbility.h"

#include "Characters/Player/StealthPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Inventory/InventoryComponent.h"

void UItemConsumableGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                     const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CachedInventoryComponent)
	{
		AStealthPlayerState* PlayerState = Cast<AStealthPlayerState>(ActorInfo->PlayerController->PlayerState);
		if (!PlayerState)
		{
			return;
		}

		CachedInventoryComponent = PlayerState->GetInventoryComponent();
	}

	if (!CachedInventoryComponent)
	{
		return;
	}

	bool bItemRemoved = CachedInventoryComponent->TryRemoveItem(NeededItem, NeededItemQuantity);

	EndAbility(Handle, ActorInfo, ActivationInfo, false, !bItemRemoved);
}

bool UItemConsumableGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                        const FGameplayTagContainer* SourceTags,
                                                        const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!CachedInventoryComponent)
	{
		AStealthPlayerState* PlayerState = Cast<AStealthPlayerState>(ActorInfo->PlayerController->PlayerState);
		if (!PlayerState)
		{
			return false;
		}

		CachedInventoryComponent = PlayerState->GetInventoryComponent();
	}

	if (!CachedInventoryComponent)
	{
		return false;
	}

	return CachedInventoryComponent->HasItem(NeededItem, NeededItemQuantity);
}
