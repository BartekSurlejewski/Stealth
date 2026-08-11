#include "UI/DialogueMenu/DialogueMenu_DialogueOption.h"

#include "Components/TextBlock.h"
#include "DialogueSystem/DialogueOption.h"

void UDialogueMenu_DialogueOption::Hide()
{
	// OptionText->SetText("");
	
	Super::Hide();
}

void UDialogueMenu_DialogueOption::SetData(const UDialogueOption* DialogueOption, const int32 NewOptionIndex)
{
	this->OptionIndex = NewOptionIndex;
	if (!OptionText || !DialogueOption)
	{
		return;
	}

	OptionText->SetText(DialogueOption->OptionText);
}
