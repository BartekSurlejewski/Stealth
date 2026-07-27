#include "DialogueSystem/DialogueController.h"

#include "GameplayTagContainer.h"
#include "Core/Executable.h"
#include "DialogueSystem/DialogueComponent.h"
#include "DialogueSystem/SpecialBehaviour/USpecialDialogueBehaviour.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/StealthMessages.h"

void UDialogueController::Initialize()
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	DialogueOptionChosenMessageListenerHandle = MsgSubsystem.RegisterListener<FIntMessage>(StealthMessageChannels::TAG_Message_Input_Dialogue_OptionChosen, this,
	                                                                                       &UDialogueController::Input_OnDialogueOptionChosen);
	DialogueEndMessageListenerHandle = MsgSubsystem.RegisterListener<FDialogueMessage>(StealthMessageChannels::TAG_Message_Input_Dialogue_End, this,
	                                                                                   &UDialogueController::Input_OnDialogueEnd);
}

void UDialogueController::Dispose() const
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(DialogueOptionChosenMessageListenerHandle);
}

bool UDialogueController::TryStartDialogue(UDialogueComponent* NewDialogueComponent)
{
	if (bIsDialogueOngoing)
	{
		return false;
	}

	NpcDialogueComponent = NewDialogueComponent;
	bIsDialogueOngoing = true;

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	const FDialogueMessage Message(NpcDialogueComponent);
	MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Dialogue_Started, Message);

	return true;
}

void UDialogueController::Input_OnDialogueOptionChosen(FGameplayTag Channel, const FIntMessage& Message)
{
	if (!bIsDialogueOngoing || !NpcDialogueComponent)
	{
		return;
	}

	UDialogueOption* SelectedOption = NpcDialogueComponent->GetDialogueOption(Message.Value);

	if (SelectedOption->bCanBeUsedOnce && SelectedOption->bUsedAlready)
	{
		return;
	}

	for (const TObjectPtr<USpecialDialogueBehaviour>& Behaviour : SelectedOption->CustomActions)
	{
		IExecutable::Execute_PerformAction(Behaviour, this);
	}

	SelectedOption->bUsedAlready = true;
}

void UDialogueController::Input_OnDialogueEnd(FGameplayTag Channel, const FDialogueMessage& Message)
{
	if (Message.DialogueComponent == NpcDialogueComponent || !NpcDialogueComponent)
	{
		NpcDialogueComponent = nullptr;
		bIsDialogueOngoing = false;

		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
		MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Dialogue_Ended, Message);
	}
}
