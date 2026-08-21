#include "Legality/LegalitySubsystem.h"

#include "AbilitySystemComponent.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "TimeSystem/DailyRegimen/DailyRegimenSubsystem.h"

void ULegalitySubsystem::Tick(float DeltaTime)
{
	TimeSinceLastTick += DeltaTime;
	if (TimeSinceLastTick < TickInterval)
	{
		return;
	}

	TimeSinceLastTick = 0.0f;
	
	Super::Tick(DeltaTime);

	AStealthPlayerCharacter* Player = UCharactersRegistrySubsystem::Get(this)->GetPlayerCharacter();
	if (!Player)
	{
		bIsPlayerPerformingIllegalAction = false;
		return;
	}

	//TODO: Add illegal action/tresspassing tags to player ASC
	// 1. Check GAS gameplay tags for illegal or suspicious states
	if (const UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
	{
		if (ASC->HasMatchingGameplayTag(StealthAiTags::TAG_Player_State_Illegal) ||
			ASC->HasMatchingGameplayTag(StealthAiTags::TAG_Player_State_Trespassing))
		{
			bIsPlayerPerformingIllegalAction = true;
			return;
		}
	}

	// 2. Check if player is performing daily routine
	if (!UDailyRegimenSubsystem::Get(this)->IsPlayerPerformingCurrentTask())
	{
		bIsPlayerPerformingIllegalAction = true;
		return;
	}

	bIsPlayerPerformingIllegalAction = false;
	return;
}
