#include "QuestManagerSubsystem.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "Conditions/QuestCondition.h"
#include "Data/QuestDefinition.h"
#include "Data/QuestSystemMessages.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Objectives/QuestObjectiveDefinition.h"
#include "Rewards/QuestReward.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuest, Log, All);

// ============================================================================
// Lifecycle
// ============================================================================

void UQuestManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// TODO: adde save/load support here
	for (const FPrimaryAssetId& ID : InitialQuestIDs)
	{
		ActivateQuest(ID);
	}
}

void UQuestManagerSubsystem::Deinitialize()
{
	QuestsInstances.Reset();

	Super::Deinitialize();
}

UQuestManagerSubsystem* UQuestManagerSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(
		WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	const UGameInstance* GameInstance = World->GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UQuestManagerSubsystem>() : nullptr;
}

// ============================================================================
// Definition access
// ============================================================================

const UQuestDefinition* UQuestManagerSubsystem::GetDefinition(FPrimaryAssetId QuestID) const
{
	UAssetManager& AM = UAssetManager::Get();

	const FSoftObjectPath AssetPath = AM.GetPrimaryAssetPath(QuestID);
	if (!AssetPath.IsValid())
	{
		UE_LOG(LogQuest, Warning, TEXT("No asset registered for %s"), *QuestID.ToString());
		return nullptr;
	}

	return Cast<UQuestDefinition>(AM.GetPrimaryAssetObject(QuestID));
}

// ============================================================================
// Quest lifetime
// ============================================================================

void UQuestManagerSubsystem::ActivateQuest(FPrimaryAssetId QuestID)
{
	const UQuestDefinition* Def = GetDefinition(QuestID);
	if (!Def)
	{
		return;
	}

	if (const FQuestInstance* Existing = QuestsInstances.Find(QuestID))
	{
		if (Existing->Status == EQuestStatus::Active || Existing->Status == EQuestStatus::Completed)
		{
			return;
		}
	}

	FQuestInstance& Instance = QuestsInstances.FindOrAdd(QuestID);
	Instance.QuestDefinitionID = QuestID;
	Instance.Status = EQuestStatus::Active;
	Instance.Objectives.Reset();
	Instance.StartedAt = FDateTime::Now();

	for (const UQuestObjectiveDefinition* Obj : Def->Objectives)
	{
		if (!Obj) continue;

		FQuestObjectiveState State;
		State.ObjectiveID = Obj->ObjectiveID;
		Instance.Objectives.Add(State);
	}

	BroadcastQuestStartedMessage(QuestID);
}

void UQuestManagerSubsystem::CompleteQuest(FPrimaryAssetId QuestID)
{
	FQuestInstance* Instance = QuestsInstances.Find(QuestID);
	if (!Instance || Instance->Status != EQuestStatus::Active)
	{
		return;
	}

	const UQuestDefinition* Def = GetDefinition(QuestID);
	if (!Def)
	{
		return;
	}

	for (const UQuestReward* Reward : Def->Rewards)
	{
		if (!Reward) continue;

		const bool bGranted = Reward->GrantReward(this);
		if (!bGranted)
		{
			UE_LOG(LogQuest, Warning, TEXT("No reward for quest: %s"),
			       *QuestID.ToString());
		}
	}

	Instance->Status = EQuestStatus::Completed;
	Instance->CompletedAt = FDateTime::Now();
	Instance->CompletionCount++;

	BroadcastQuestCompletedMessage(QuestID);

	RefreshLockedQuests();

	if (Def->bIsRepeatable)
	{
		// TODO: reactivate after cooldown
	}
}

void UQuestManagerSubsystem::FailQuest(FPrimaryAssetId QuestID)
{
	FQuestInstance* Instance = QuestsInstances.Find(QuestID);
	if (!Instance || Instance->Status != EQuestStatus::Active)
	{
		return;
	}

	Instance->Status = EQuestStatus::Failed;
	BroadcastQuestFailedMessage(QuestID);
}

void UQuestManagerSubsystem::AbandonQuest(FPrimaryAssetId QuestID)
{
	FQuestInstance* Instance = QuestsInstances.Find(QuestID);
	if (!Instance || Instance->Status != EQuestStatus::Active)
	{
		return;
	}

	Instance->Status = EQuestStatus::Abandoned;
}

void UQuestManagerSubsystem::ForceCompleteQuest(FPrimaryAssetId QuestID)
{
	FQuestInstance& Instance = QuestsInstances.FindOrAdd(QuestID);
	Instance.QuestDefinitionID = QuestID;
	Instance.Status = EQuestStatus::Active;

	if (const UQuestDefinition* Def = GetDefinition(QuestID))
	{
		for (FQuestObjectiveState& ObjState : Instance.Objectives)
		{
			ObjState.bIsCompleted = true;
		}
	}

	CompleteQuest(QuestID);
}

void UQuestManagerSubsystem::ForceUnlockQuest(FPrimaryAssetId QuestID)
{
	FQuestInstance& Instance = QuestsInstances.FindOrAdd(QuestID);
	Instance.QuestDefinitionID = QuestID;

	if (Instance.Status == EQuestStatus::Locked)
	{
		Instance.Status = EQuestStatus::Available;
		BroadcastQuestUnlockedMessage(QuestID);
	}
}

// ============================================================================
// Queries
// ============================================================================

EQuestStatus UQuestManagerSubsystem::GetQuestStatus(FPrimaryAssetId QuestID) const
{
	const FQuestInstance* Instance = QuestsInstances.Find(QuestID);
	return Instance ? Instance->Status : EQuestStatus::Locked;
}

bool UQuestManagerSubsystem::IsQuestCompleted(FPrimaryAssetId QuestID) const
{
	return GetQuestStatus(QuestID) == EQuestStatus::Completed;
}

bool UQuestManagerSubsystem::IsQuestActive(FPrimaryAssetId QuestID) const
{
	return GetQuestStatus(QuestID) == EQuestStatus::Active;
}

bool UQuestManagerSubsystem::IsQuestLocked(FPrimaryAssetId QuestID) const
{
	return GetQuestStatus(QuestID) == EQuestStatus::Locked;
}

bool UQuestManagerSubsystem::GetQuestInstance(FPrimaryAssetId QuestID, FQuestInstance& OutInstance) const
{
	if (const FQuestInstance* Instance = QuestsInstances.Find(QuestID))
	{
		OutInstance = *Instance;
		return true;
	}
	return false;
}

TArray<FPrimaryAssetId> UQuestManagerSubsystem::GetActiveQuestIDs() const
{
	return GetQuestIDsByStatus(EQuestStatus::Active);
}

TArray<FPrimaryAssetId> UQuestManagerSubsystem::GetQuestIDsByStatus(EQuestStatus Status) const
{
	TArray<FPrimaryAssetId> Result;
	for (const auto& [ID, Instance] : QuestsInstances)
	{
		if (Instance.Status == Status)
		{
			Result.Add(ID);
		}
	}
	return Result;
}


// ============================================================================
// Conditions evaluation
// ============================================================================

bool UQuestManagerSubsystem::EvaluateFilterConditions(
	const TArray<UQuestCondition*>& Conditions, const FQuestEventMessage& Msg) const
{
	for (const UQuestCondition* Cond : Conditions)
	{
		if (Cond && !Cond->IsConditionMet(this))
		{
			return false;
		}
	}
	return true;
}

bool UQuestManagerSubsystem::EvaluateUnlockConditions(const UQuestDefinition* Def) const
{
	if (!Def)
	{
		return false;
	}

	for (const UQuestCondition* Cond : Def->UnlockConditions)
	{
		if (Cond && !Cond->IsConditionMet(this))
		{
			return false;
		}
	}
	return true;
}

bool UQuestManagerSubsystem::IsQuestComplete(const UQuestDefinition* Def, const FQuestInstance& Instance) const
{
	if (!Def)
	{
		return false;
	}

	for (int32 i = 0; i < Def->Objectives.Num(); ++i)
	{
		const bool bOptional = Def->Objectives[i] && Def->Objectives[i]->bIsOptional;
		if (!bOptional && Instance.Objectives.IsValidIndex(i) && !Instance.Objectives[i].bIsCompleted)
		{
			return false;
		}
	}
	return true;
}

void UQuestManagerSubsystem::RefreshLockedQuests()
{
	TArray<FPrimaryAssetId> LockedIDs = GetQuestIDsByStatus(EQuestStatus::Locked);

	for (const FPrimaryAssetId& QuestID : LockedIDs)
	{
		const UQuestDefinition* Def = GetDefinition(QuestID);
		if (!Def)
		{
			continue;
		}

		if (EvaluateUnlockConditions(Def))
		{
			FQuestInstance& Instance = QuestsInstances.FindOrAdd(QuestID);
			Instance.QuestDefinitionID = QuestID;
			Instance.Status = EQuestStatus::Available;
			BroadcastQuestUnlockedMessage(QuestID);
		}
	}
}

// ============================================================================
// Player tags
// ============================================================================

UAbilitySystemComponent* UQuestManagerSubsystem::GetPlayerASC() const
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || !PC->PlayerState)
	{
		return nullptr;
	}
	return PC->PlayerState->FindComponentByClass<UAbilitySystemComponent>();
}

void UQuestManagerSubsystem::GrantTagToPlayer(FGameplayTag Tag)
{
	if (!Tag.IsValid())
	{
		return;
	}

	GrantedQuestTags.AddTag(Tag);

	if (UAbilitySystemComponent* ASC = GetPlayerASC())
	{
		ASC->AddLooseGameplayTag(Tag);
	}

	RefreshLockedQuests();
}

bool UQuestManagerSubsystem::HasPlayerTag(FGameplayTag Tag) const
{
	if (const UAbilitySystemComponent* ASC = GetPlayerASC())
	{
		return ASC->HasMatchingGameplayTag(Tag);
	}
	return GrantedQuestTags.HasTag(Tag);
}

// ============================================================================
// Broadcasts
// ============================================================================

void UQuestManagerSubsystem::BroadcastQuestStartedMessage(FPrimaryAssetId QuestID) const
{
	FQuestStartedMessage Msg;
	Msg.QuestID = QuestID;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage<FQuestStartedMessage>(QuestMessageChannels::TAG_Quest_Started.GetTag(), Msg);
}

void UQuestManagerSubsystem::BroadcastObjectiveUpdatedMessage(FPrimaryAssetId QuestID, FName ObjectiveID) const
{
	FQuestObjectiveUpdatedMessage Msg;
	Msg.QuestID = QuestID;
	Msg.ObjectiveID = ObjectiveID;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage<FQuestObjectiveUpdatedMessage>(QuestMessageChannels::TAG_Quest_ObjectiveUpdated.GetTag(), Msg);
}

void UQuestManagerSubsystem::BroadcastQuestCompletedMessage(FPrimaryAssetId QuestID) const
{
	FQuestCompletedMessage Msg;
	Msg.QuestID = QuestID;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage<FQuestCompletedMessage>(QuestMessageChannels::TAG_Quest_Completed.GetTag(), Msg);
}

void UQuestManagerSubsystem::BroadcastQuestFailedMessage(FPrimaryAssetId QuestID) const
{
	FQuestFailedMessage Msg;
	Msg.QuestID = QuestID;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage<FQuestFailedMessage>(QuestMessageChannels::TAG_Quest_Failed.GetTag(), Msg);
}

void UQuestManagerSubsystem::BroadcastQuestUnlockedMessage(FPrimaryAssetId QuestID) const
{
	FQuestUnlockedMessage Msg;
	Msg.QuestID = QuestID;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage<FQuestUnlockedMessage>(QuestMessageChannels::TAG_Quest_Unlocked.GetTag(), Msg);
}

// ============================================================================
// Save / Load
// ============================================================================

void UQuestManagerSubsystem::ExportSaveData(FQuestSaveData& OutData) const
{
	OutData.QuestIDs.Reset();
	OutData.Instances.Reset();

	for (const auto& [ID, Instance] : QuestsInstances)
	{
		OutData.QuestIDs.Add(ID);
		OutData.Instances.Add(Instance);
	}

	OutData.GrantedQuestTags = GrantedQuestTags;
}

void UQuestManagerSubsystem::ImportSaveData(const FQuestSaveData& Data)
{
	QuestsInstances.Reset();

	GrantedQuestTags = Data.GrantedQuestTags;

	if (UAbilitySystemComponent* ASC = GetPlayerASC())
	{
		for (const FGameplayTag& Tag : Data.GrantedQuestTags)
		{
			ASC->AddLooseGameplayTag(Tag);
		}
	}

	for (int32 i = 0; i < Data.QuestIDs.Num(); ++i)
	{
		const FPrimaryAssetId& ID = Data.QuestIDs[i];
		QuestsInstances.Add(ID, Data.Instances[i]);

		if (Data.Instances[i].Status == EQuestStatus::Active)
		{
			if (const UQuestDefinition* Def = GetDefinition(ID))
			{
				for (int32 ObjIdx = 0; ObjIdx < Def->Objectives.Num(); ++ObjIdx)
				{
					const UQuestObjectiveDefinition* ObjDef = Def->Objectives[ObjIdx];
					const bool bAlreadyDone = Data.Instances[i].Objectives.IsValidIndex(ObjIdx)
						&& Data.Instances[i].Objectives[ObjIdx].bIsCompleted;

					// TODO: Activate quest
				}
			}
		}
	}
}
