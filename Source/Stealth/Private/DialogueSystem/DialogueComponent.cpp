#include "DialogueSystem/DialogueComponent.h"

#include "Characters/NPCs/NpcCharacter.h"

UDialogueComponent::UDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UDialogueOption* UDialogueComponent::GetDialogueOption(int32 OptionIndex)
{
	if (!DialogueOptions.IsValidIndex(OptionIndex))
	{
		return nullptr;
	}

	return DialogueOptions[OptionIndex];
}

ANpcCharacter* UDialogueComponent::GetOwningNpc() const
{
	return Cast<ANpcCharacter>(GetOwner());
}
