#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "QuestInstance.generated.h"

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
	Locked,
	Available,
	Active,
	Completed,
	Failed,
	Abandoned
};

USTRUCT(BlueprintType)
struct FQuestObjectiveState
{
	GENERATED_BODY()

	UPROPERTY()
	FName ObjectiveID;
	UPROPERTY()
	int32 CurrentCount = 0; // e.g. 3/5 herbs gathered
	UPROPERTY()
	bool bIsCompleted = false;
	UPROPERTY()
	bool bIsFailed = false;
	UPROPERTY()
	bool bIsVisible = true; // hidden objectives, unveiled dynamically
};

USTRUCT(BlueprintType)
struct STEALTHQUESTSYSTEM_API FQuestInstance
{
	GENERATED_BODY()

	UPROPERTY()
	FPrimaryAssetId QuestDefinitionID;
	UPROPERTY()
	EQuestStatus Status = EQuestStatus::Locked;
	UPROPERTY()
	TArray<FQuestObjectiveState> Objectives;
	UPROPERTY()
	int32 CurrentObjectiveIndex = 0;
	UPROPERTY()
	FDateTime StartedAt;
	UPROPERTY()
	FDateTime CompletedAt;
	UPROPERTY()
	int32 CompletionCount = 0;
};

USTRUCT(BlueprintType)
struct FQuestSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPrimaryAssetId> QuestIDs;
	UPROPERTY()
	TArray<FQuestInstance> Instances;
	UPROPERTY()
	FGameplayTagContainer GrantedQuestTags; // Tags granted by quests system
};
