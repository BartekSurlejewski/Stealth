#include "DialogueSystem/SpecialBehaviour/SpecialDialogueBehaviour_StartQuest.h"

#include "QuestManagerSubsystem.h"
#include "DialogueSystem/DialogueComponent.h"

void USpecialDialogueBehaviour_StartQuest::PerformAction_Implementation(const UObject* WorldContextObject)
{
	auto QuestManager = UQuestManagerSubsystem::Get(this);
	if (!QuestManager)
	{
		return;
	}

	const UDialogueComponent* DialogueComponent = Cast<UDialogueComponent>(WorldContextObject);
	AActor* Instigator = nullptr;
	if (DialogueComponent)
	{
		Instigator = DialogueComponent->GetOwner();
	}

	QuestManager->ActivateQuest(QuestToStart, Instigator);
}
