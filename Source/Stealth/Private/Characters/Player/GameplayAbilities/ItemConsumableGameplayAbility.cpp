#include "Characters/Player/GameplayAbilities/ItemConsumableGameplayAbility.h"

#include "Characters/Player/StealthPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Inventory/InventoryComponent.h"

void UItemConsumableGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bWasCancelled)
	{
		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
		return;
	}

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

	CachedInventoryComponent->TryRemoveItem(NeededItem, NeededItemQuantity);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
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
