#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "StealthMessages.generated.h"

class UDialogueComponent;
class AActor;
enum class ETimeOfDay : uint8;
class UDailyRegimenTask;

namespace StealthMessageChannels
{
	// Player
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Player_LookedAtInteractable);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Player_IsInRestrictedAreaChanged);

	// Input
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Input_OpenWidget);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Input_CloseWidget);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Input_DetailsMenu_Next);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Input_DetailsMenu_Prev);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Input_DetailsMenu_Prev);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Input_Dialogue_OptionChosen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Input_Dialogue_Start);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Input_Dialogue_End);

	// Daily Tasks
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_DailyTask_OnDailyTaskStarted);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_DailyTask_OnDailyTaskEnded);

	// Time
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Time_TimeChanged);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Time_TimeOfDayChanged);

	// Dialogue
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Dialogue_Started);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Dialogue_Ended);
}

USTRUCT(BlueprintType)
struct FBooleanMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	bool bValue = false;
};

USTRUCT(BlueprintType)
struct FIntMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 Value = -1;
};

USTRUCT(BlueprintType)
struct FInteractableMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	AActor* InteractableActor;
};

USTRUCT(BlueprintType)
struct FDailyTaskStartedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	UDailyRegimenTask* StartedTask;
};

USTRUCT(BlueprintType)
struct FDailyTaskEndedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	UDailyRegimenTask* EndedTask;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	bool bPlayerSucceeded;
};

USTRUCT(BlueprintType)
struct FTimeChangedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 NewHour;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 NewMinute;
};

USTRUCT(BlueprintType)
struct FTimeOfDayChangedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	ETimeOfDay NewTimeOfDay;
};

USTRUCT(BlueprintType)
struct FGameplayTagMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	FGameplayTag GameplayTag;
};

USTRUCT(BlueprintType)
struct FInputMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	FGameplayTag GameplayTag;
};

USTRUCT(BlueprintType)
struct FDialogueMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	UDialogueComponent* DialogueComponent;
};
