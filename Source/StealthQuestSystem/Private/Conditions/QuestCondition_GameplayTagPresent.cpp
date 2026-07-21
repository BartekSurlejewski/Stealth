#include "Conditions/QuestCondition_GameplayTagPresent.h"

#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

bool UQuestCondition_GameplayTagPresent::IsConditionMet_Implementation(const UObject* WCO) const
{
	if (!RequiredTag.IsValid())
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = GetPlayerASC(WCO);
	if (!ASC)
	{
		return false;
	}

	return bExactMatch ? ASC->HasMatchingGameplayTag(RequiredTag) : ASC->GetOwnedGameplayTags().HasTag(RequiredTag);
}

const UAbilitySystemComponent* UQuestCondition_GameplayTagPresent::GetPlayerASC(const UObject* WorldContextObject)
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	if (!PC || !PC->PlayerState)
	{
		return nullptr;
	}

	return PC->PlayerState->FindComponentByClass<UAbilitySystemComponent>();
}
