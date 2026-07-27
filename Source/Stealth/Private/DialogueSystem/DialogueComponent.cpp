#include "DialogueSystem/DialogueComponent.h"


UDialogueComponent::UDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDialogueComponent::SelectDialogueOption(int32 OptionIndex, ACharacter* PlayerCharacter)
{
	if (!DialogueOptions.IsValidIndex(OptionIndex))
	{
		return;
	}

	const FDialogueOption& SelectedOption = DialogueOptions[OptionIndex];

	// Trigger quest if needed
	if (!SelectedOption.QuestToGrant.IsValid())
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Dialogue] Granting quest: %s"), *SelectedOption.QuestToGrant.ToString());

	OnDialogueOptionSelected.Broadcast(SelectedOption.QuestToGrant);
}
