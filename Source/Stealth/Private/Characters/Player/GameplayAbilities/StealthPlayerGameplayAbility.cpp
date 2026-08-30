#include "Characters/Player/GameplayAbilities/StealthPlayerGameplayAbility.h"

const FGameplayAbilityStealthPlayerInfo* UStealthPlayerGameplayAbility::GetStealthPlayerInfo() const
{
	return static_cast<const FGameplayAbilityStealthPlayerInfo*>(CurrentActorInfo);
}

AStealthPlayerCharacter* UStealthPlayerGameplayAbility::GetPlayerCharacter() const
{
	if (const FGameplayAbilityStealthPlayerInfo* Info = GetStealthPlayerInfo())
	{
		return Info->StealthPlayerCharacter.Get();
	}
	return nullptr;
}
