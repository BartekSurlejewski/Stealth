#include "UI/DialogueMenu/DialogueMenu.h"

#include "Characters/NPCs/NpcCharacter.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "DialogueSystem/DialogueComponent.h"
#include "DialogueSystem/DialogueOption.h"
#include "Messages/StealthMessages.h"

void UDialogueMenu::NativeConstruct()
{
	Super::NativeConstruct();

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	DialogueStartedListenerHandle = MsgSubsystem.RegisterListener<FDialogueMessage>(
		StealthMessageChannels::TAG_Message_Dialogue_Started,
		this,
		&UDialogueMenu::OnDialogueStarted);

	DialogueEndedListenerHandle = MsgSubsystem.RegisterListener<FDialogueMessage>(
		StealthMessageChannels::TAG_Message_Dialogue_Ended,
		this,
		&UDialogueMenu::OnDialogueEnded);

	SetVisibility(ESlateVisibility::Collapsed);
}

void UDialogueMenu::NativeDestruct()
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(DialogueStartedListenerHandle);
	MsgSubsystem.UnregisterListener(DialogueEndedListenerHandle);

	Super::NativeDestruct();
}

void UDialogueMenu::ShowDialogueOptions(const TArray<UDialogueOption*> NewDialogueOptions)
{
	CurrentDialogueOptions = NewDialogueOptions;

	if (!OptionsContainer)
	{
		return;
	}

	OptionsContainer->ClearChildren();

	for (int32 i = 0; i < NewDialogueOptions.Num(); ++i)
	{
		UDialogueOption* Option = NewDialogueOptions[i];
		if (!Option)
			continue;

		// Create option button
		UButton* OptionButton = NewObject<UButton>(OptionsContainer);

		// Create text
		UTextBlock* OptionText = NewObject<UTextBlock>(OptionButton);
		OptionText->SetText(Option->OptionText);

		OptionButton->AddChild(OptionText);
		OptionsContainer->AddChild(OptionButton);

		// Bind click
		OptionButton->OnClicked.AddDynamic(this, &UDialogueMenu::OnOptionButtonClicked);
	}

	SetVisibility(ESlateVisibility::Visible);
}

void UDialogueMenu::HideDialogueMenu()
{
	if (OptionsContainer)
	{
		OptionsContainer->ClearChildren();
	}

	CurrentDialogueOptions.Empty();
	SetVisibility(ESlateVisibility::Collapsed);
}

void UDialogueMenu::OnDialogueStarted(FGameplayTag Channel, const FDialogueMessage& Message)
{
	if (!Message.DialogueComponent)
	{
		return;
	}

	// Show NPC name
	ANpcCharacter* NPC = Message.DialogueComponent->GetOwningNpc();
	if (NPC && NpcNameText)
	{
		NpcNameText->SetText(FText::FromString(NPC->GetName()));
	}

	// Show greeting
	if (NPCGreetingText)
	{
		NPCGreetingText->SetText(Message.DialogueComponent->GetNpcGreeting());
	}

	// Show options
	ShowDialogueOptions(Message.DialogueComponent->GetAllDialogueOptions());
}

void UDialogueMenu::OnDialogueEnded(FGameplayTag Channel, const FDialogueMessage& Message)
{
	HideDialogueMenu();
}

void UDialogueMenu::OnOptionButtonClicked()
{
	// Find which button was clicked
	if (!OptionsContainer)
	{
		return;
	}

	TArray<UWidget*> Children = OptionsContainer->GetAllChildren();
	for (int32 i = 0; i < Children.Num(); ++i)
	{
		if (const UButton* Button = Cast<UButton>(Children[i]))
		{
			if (Button->IsHovered()) // This button was clicked
			{
				// Broadcast option selection
				UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
				FIntMessage SelectionMsg;
				SelectionMsg.Value = i;
				MsgSubsystem.BroadcastMessage(
					StealthMessageChannels::TAG_Message_Input_Dialogue_OptionChosen,
					SelectionMsg);
				return;
			}
		}
	}
}
