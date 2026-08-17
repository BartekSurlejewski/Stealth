#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "Inventory/Items/InventoryItem.h"
#include "UI/Core/StealthUIData.h"
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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Input_ToggleWidget);
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

	// Inventory
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Inventory_ItemAdded);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Inventory_ItemRemoved);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_Inventory_ItemMoved);

	// Interaction
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_InteractionPerformed);

	// UI
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_UI_WidgetShown);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Message_UI_WidgetHidden);
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

USTRUCT(BlueprintType)
struct FPlayerInventoryItemAddedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	TObjectPtr<UItemDefinition> AddedItem;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 AddedQuantity;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 QuantityInInventory;
};

USTRUCT(BlueprintType)
struct FPlayerInventoryItemRemovedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	TObjectPtr<UItemDefinition> RemovedItem;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 RemovedQuantity;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 QuantityInInventory;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	bool bShouldDrop;
};

USTRUCT(BlueprintType)
struct FNpcInventoryItemAddedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	FGuid NpcGUID;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	TObjectPtr<UItemDefinition> AddedItem;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 AddedQuantity;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 QuantityInInventory;
};

USTRUCT(BlueprintType)
struct FNpcInventoryItemRemovedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	FGuid NpcGUID;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	TObjectPtr<UItemDefinition> RemovedItem;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 RemovedQuantity;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 QuantityInInventory;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	bool bShouldDrop;
};

USTRUCT(BlueprintType)
struct FItemMovedFromNpcToPlayerMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	FGuid NpcGUID;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	TObjectPtr<UItemDefinition> Moved;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 MovedQuantity;
};

USTRUCT(BlueprintType)
struct FItemMovedFromPlayerToNpcMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	FGuid NpcGUID;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	TObjectPtr<UItemDefinition> Moved;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 MovedQuantity;
};

USTRUCT(BlueprintType)
struct FInteractionNotifyMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	FGameplayTag InteractionTag;
};

USTRUCT(BlueprintType)
struct FWidgetToggleMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	EUILayer WidgetLayer;
};
