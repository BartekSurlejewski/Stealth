#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "UObject/Object.h"
#include "QuestSystemMessages.generated.h"

enum class EQuestStatus : uint8;
enum class EQuestObjectiveStatus : uint8;

USTRUCT(BlueprintType)
struct FQuestStatusChangedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	FPrimaryAssetId QuestID;
	UPROPERTY(BlueprintReadWrite)
	EQuestStatus QuestStatus;
};

USTRUCT(BlueprintType)
struct FQuestObjectiveMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	FPrimaryAssetId QuestId;
	UPROPERTY(BlueprintReadWrite)
	FName ObjectiveID;
};

USTRUCT(BlueprintType)
struct FQuestObjectiveUpdatedMessage : public FQuestObjectiveMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	EQuestObjectiveStatus Status;
};

namespace QuestMessageChannels
{
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Quest_Started);
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Quest_ObjectiveUpdated);
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Quest_Completed);
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Quest_Failed);
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Quest_Unlocked);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Quest_Status_Changed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Quest_Objective_Status_Changed);
}
