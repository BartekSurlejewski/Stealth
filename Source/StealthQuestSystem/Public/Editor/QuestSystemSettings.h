#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "QuestSystemSettings.generated.h"

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Quest System"))
class STEALTHQUESTSYSTEM_API UQuestSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UQuestSystemSettings();

	UPROPERTY(EditDefaultsOnly, Config, Category = "Quest", meta = (AllowedTypes = "QuestDefinition"))
	TArray<FPrimaryAssetId> InitialQuestIDs;
};
