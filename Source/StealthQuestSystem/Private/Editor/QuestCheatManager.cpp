#if WITH_EDITOR
#include "Editor/QuestCheatManager.h"

#include "QuestManagerSubsystem.h"

void UQuestCheatManager::Quest_ForceComplete(FString QuestName)
{
	UQuestManagerSubsystem* QM = UQuestManagerSubsystem::Get(this);
	QM->ForceCompleteQuest(FPrimaryAssetId(TEXT("Quest"), FName(*QuestName)));
}
void UQuestCheatManager::Quest_ForceUnlock(FString QuestName)
{
	UQuestManagerSubsystem::Get(this)->ForceUnlockQuest(
			FPrimaryAssetId(TEXT("Quest"), FName(*QuestName)));
}
void UQuestCheatManager::Quest_ListActive() {}
#endif
