#include "DialogueSystem/SpecialBehaviour/SpecialDialogueBehaviour_StartQuest.h"

#include "QuestManagerSubsystem.h"

void USpecialDialogueBehaviour_StartQuest::PerformAction_Implementation(const UObject* WorldContextObject)
{
	auto QuestManager = UQuestManagerSubsystem::Get(this);
	if (!QuestManager)
	{
		return;
	}

	QuestManager->ActivateQuest(QuestToStart);
}
