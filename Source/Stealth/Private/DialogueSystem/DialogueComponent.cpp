#include "DialogueSystem/DialogueComponent.h"

#include "DialogueSystem/SpecialBehaviour/USpecialDialogueBehaviour.h"


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
