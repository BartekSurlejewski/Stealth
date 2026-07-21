#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "UObject/Object.h"
#include "QuestSystemMessages.generated.h"

USTRUCT(BlueprintType)
struct FQuestEventMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FPrimaryAssetId ItemID;
	UPROPERTY(BlueprintReadWrite)
	FName TargetID;
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> Instigator;

	UPROPERTY(BlueprintReadWrite)
	int32 Count = 1;
};

USTRUCT(BlueprintType)
struct FQuestStartedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	FPrimaryAssetId QuestID;
};

USTRUCT(BlueprintType)
struct FQuestObjectiveUpdatedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	FPrimaryAssetId QuestID;
	UPROPERTY(BlueprintReadWrite)
	FName ObjectiveID;
};

USTRUCT(BlueprintType)
struct FQuestCompletedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	FPrimaryAssetId QuestID;
};

USTRUCT(BlueprintType)
struct FQuestFailedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	FPrimaryAssetId QuestID;
};

USTRUCT(BlueprintType)
struct FQuestUnlockedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite)
	FPrimaryAssetId QuestID;
};

namespace QuestMessageChannels
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Quest_Started);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Quest_ObjectiveUpdated);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Quest_Completed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Quest_Failed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Quest_Unlocked);
}
