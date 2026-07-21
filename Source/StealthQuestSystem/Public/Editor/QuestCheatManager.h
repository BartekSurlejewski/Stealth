
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "QuestCheatManager.generated.h"


UCLASS()
class STEALTHQUESTSYSTEM_API UQuestCheatManager : public UCheatManager
{
	GENERATED_BODY()
public:
	UFUNCTION(Exec)
	void Quest_ForceComplete(FString QuestName);
	UFUNCTION(Exec)
	void Quest_ForceUnlock(FString QuestName);
	UFUNCTION(Exec)
	void Quest_ListActive();
};
