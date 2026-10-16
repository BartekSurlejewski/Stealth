#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcCharacter.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Components/StateTreeAIComponent.h"

UNpcContextComponent::UNpcContextComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentStateTag = StealthAiTags::TAG_NPC_State_Unaware;
}

void UNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const UCharactersRegistrySubsystem* Registry = UCharactersRegistrySubsystem::Get(this))
	{
		if (ANpcAiController* Controller = GetNpcController())
		{
			OwnerNpcGuid = Registry->GetNpcGuidByController(Controller);
		}
		else if (ANpcCharacter* NpcChar = Cast<ANpcCharacter>(GetOwner()))
		{
			OwnerNpcGuid = Registry->GetNpcGuidByCharacter(NpcChar);
		}
	}
}

UStateTreeAIComponent* UNpcContextComponent::GetStateTreeComponent() const
{
	if (!CachedStateTreeComponent.IsValid())
	{
		if (const ANpcAiController* Controller = GetNpcController())
		{
			CachedStateTreeComponent = Controller->FindComponentByClass<UStateTreeAIComponent>();
		}
		else if (GetOwner())
		{
			CachedStateTreeComponent = GetOwner()->FindComponentByClass<UStateTreeAIComponent>();
		}
	}
	return CachedStateTreeComponent.Get();
}

void UNpcContextComponent::SetCurrentStateTag(const FGameplayTag& NewStateTag)
{
	if (CurrentStateTag != NewStateTag)
	{
		const FGameplayTag OldTag = CurrentStateTag;
		CurrentStateTag = NewStateTag;
		OnNpcStateChanged.Broadcast(CurrentStateTag, OldTag);
	}
}

void UNpcContextComponent::SetCurrentStanceTag(const FGameplayTag& NewStanceTag)
{
	CurrentStanceTag = NewStanceTag;
}

void UNpcContextComponent::SendStateTreeEvent(const FGameplayTag& EventTag)
{
	if (!EventTag.IsValid())
	{
		return;
	}

	if (UStateTreeAIComponent* StateTreeComp = GetStateTreeComponent())
	{
		StateTreeComp->SendStateTreeEvent(EventTag);
	}
}

void UNpcContextComponent::HandleCrimeReported(const FAiCrimeEventPayload& CrimePayload)
{
	SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_CrimeReported);
}
