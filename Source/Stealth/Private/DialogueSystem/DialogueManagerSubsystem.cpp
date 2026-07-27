#include "DialogueSystem/DialogueManagerSubsystem.h"

#include "DialogueSystem/DialogueController.h"
#include "Stealth/Stealth.h"

void UDialogueManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	DialogueController = NewObject<UDialogueController>(this);
	DialogueController->Initialize();
}

void UDialogueManagerSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
	DialogueController->Dispose();
	DialogueController = nullptr;

	Super::OnWorldEndPlay(InWorld);
}

bool UDialogueManagerSubsystem::TryStartDialogue(UDialogueComponent* NpcDialogueComponent)
{
	if (!DialogueController)
	{
		UE_LOG(LogStealth, Error, TEXT("[UDialogueManagerSubsystem] DialogueController is nullptr"));

		return false;
	}

	return DialogueController->TryStartDialogue(NpcDialogueComponent);
}
