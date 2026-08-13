#include "QuestManagerSubsystem.h"
#include "Engine/AssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "Conditions/QuestCondition.h"
#include "Data/QuestDefinition.h"
#include "Data/QuestSystemMessages.h"
#include "Editor/QuestSystemSettings.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Objectives/QuestObjective.h"
#include "Rewards/QuestReward.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuest, Log, All);

// ============================================================================
// Lifecycle
// ============================================================================

void UQuestManagerSubsystem::PlayerInitialize()
{
	QuestObjectiveStatusChangedHandle = UGameplayMessageSubsystem::Get(this).RegisterListener<FQuestObjectiveUpdatedMessage>(
		QuestMessageChannels::TAG_Quest_Objective_Status_Changed.GetTag(), this,
		&UQuestManagerSubsystem::OnObjectiveStatusChanged);

	const UQuestSystemSettings* Settings = GetDefault<UQuestSystemSettings>();
	check(Settings);

	UE_LOG(LogTemp, Warning, TEXT("InitialQuestIDs count: %d"), Settings->InitialQuestIDs.Num());

	// TODO: adde save/load support here
	for (const FPrimaryAssetId& QuestID : Settings->InitialQuestIDs)
	{
		if (!QuestID.IsValid())
		{
			continue;
		}

		ActivateQuest(QuestID, nullptr);
	}
}

void UQuestManagerSubsystem::Deinitialize()
{
	QuestsInstances.Reset();

	UGameplayMessageSubsystem::Get(this).UnregisterListener(QuestObjectiveStatusChangedHandle);

	Super::Deinitialize();
}

UQuestManagerSubsystem* UQuestManagerSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
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

const UQuestDefinition* UQuestManagerSubsystem::GetQuestDefinition(const FPrimaryAssetId& QuestID) const
{
	UAssetManager& AM = UAssetManager::Get();

	const FSoftObjectPath AssetPath = AM.GetPrimaryAssetPath(QuestID);
	if (!AssetPath.IsValid())
	{
		UE_LOG(LogQuest, Warning, TEXT("No asset registered for %s"), *QuestID.ToString());
		return nullptr;
	}

	UObject* LoadedAsset = AM.GetStreamableManager().LoadSynchronous(AssetPath);
	return Cast<UQuestDefinition>(LoadedAsset);
}

// ============================================================================
// Quest lifetime
// ============================================================================

UQuestInstance* UQuestManagerSubsystem::ActivateQuest(const FPrimaryAssetId QuestID, AActor* Instigator)
{
	const UQuestDefinition* Def = GetQuestDefinition(QuestID);
	if (!Def)
	{
		return nullptr;
	}

	if (const UQuestInstance* Existing = QuestsInstances.FindRef(QuestID))
	{
		if (Existing->GetQuestStatus() == EQuestStatus::Active || Existing->GetQuestStatus() == EQuestStatus::Finished)
		{
			return nullptr;
		}
	}

	UQuestInstance* RuntimeQuest = NewObject<UQuestInstance>(this);
	QuestsInstances.Add(QuestID, RuntimeQuest);

	TArray<TObjectPtr<UQuestObjective>> Objectives;

	for (const UQuestObjective* TemplateObjective : Def->Objectives)
	{
		if (!TemplateObjective)
		{
			continue;
		}

		UQuestObjective* RuntimeObjective = DuplicateObject<UQuestObjective>(TemplateObjective, this);
		Objectives.Add(RuntimeObjective);
	}

	RuntimeQuest->Initialize(QuestID, EQuestStatus::Active, MoveTemp(Objectives), Instigator);
	RuntimeQuest->ActivateQuest();
	RefreshLockedObjectives();
	BroadcastQuestStatusChangedMessage(QuestID, RuntimeQuest->GetQuestStatus());

	UE_LOG(LogQuest, Warning, TEXT("Quest Activated: %s"), *QuestID.ToString());

	return RuntimeQuest;
}

void UQuestManagerSubsystem::CompleteQuest(FPrimaryAssetId QuestID)
{
	UQuestInstance* Instance = QuestsInstances.FindRef(QuestID);
	if (!Instance || Instance->GetQuestStatus() != EQuestStatus::Active)
	{
		return;
	}

	const UQuestDefinition* Def = GetQuestDefinition(QuestID);
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

	Instance->SetStatus(EQuestStatus::Finished);

	BroadcastQuestStatusChangedMessage(QuestID, Instance->GetQuestStatus());

	RefreshLockedQuests();

	UE_LOG(LogQuest, Warning, TEXT("Quest Completed: %s"), *QuestID.ToString());

	if (Def->bIsRepeatable)
	{
		// TODO: reactivate after cooldown
	}
}

void UQuestManagerSubsystem::FailQuest(FPrimaryAssetId QuestID)
{
	UQuestInstance* Instance = QuestsInstances.FindRef(QuestID);
	if (!Instance || Instance->GetQuestStatus() != EQuestStatus::Active)
	{
		return;
	}

	Instance->SetStatus(EQuestStatus::Finished);
	BroadcastQuestStatusChangedMessage(QuestID, Instance->GetQuestStatus());
}

void UQuestManagerSubsystem::AbandonQuest(FPrimaryAssetId QuestID)
{
	UQuestInstance* Instance = QuestsInstances.FindRef(QuestID);
	if (!Instance || Instance->GetQuestStatus() != EQuestStatus::Active)
	{
		return;
	}

	Instance->SetStatus(EQuestStatus::Finished);
}

void UQuestManagerSubsystem::ForceCompleteQuest(FPrimaryAssetId QuestID)
{
	UQuestInstance* Instance = QuestsInstances.FindRef(QuestID);
	if (!Instance)
	{
		Instance = ActivateQuest(QuestID, nullptr);

		if (!Instance)
		{
			return;
		}
	}

	for (UQuestObjective* Objective : Instance->GetObjectives())
	{
		if (!Objective)
		{
			continue;
		}

		Objective->ForceCompleteObjective();
	}

	CompleteQuest(QuestID);
}

void UQuestManagerSubsystem::ForceUnlockQuest(FPrimaryAssetId QuestID)
{
	UQuestInstance* Instance = QuestsInstances.FindRef(QuestID);
	if (!Instance)
	{
		ActivateQuest(QuestID, nullptr);
	}
}

// ============================================================================
// Queries
// ============================================================================

const TArray<UQuestInstance*> UQuestManagerSubsystem::GetQuestsInstances() const
{
	TArray<UQuestInstance*> Result;
	for (const auto& [ID, Instance] : QuestsInstances)
	{
		Result.Add(Instance);
	}

	return Result;
}

EQuestStatus UQuestManagerSubsystem::GetQuestStatus(FPrimaryAssetId QuestID) const
{
	const UQuestInstance* Instance = QuestsInstances.FindRef(QuestID);
	return Instance ? Instance->GetQuestStatus() : EQuestStatus::Inactive;
}

bool UQuestManagerSubsystem::IsQuestCompleted(FPrimaryAssetId QuestID) const
{
	return GetQuestStatus(QuestID) == EQuestStatus::Finished;
}

bool UQuestManagerSubsystem::IsQuestActive(FPrimaryAssetId QuestID) const
{
	return GetQuestStatus(QuestID) == EQuestStatus::Active;
}

bool UQuestManagerSubsystem::IsQuestLocked(FPrimaryAssetId QuestID) const
{
	return GetQuestStatus(QuestID) == EQuestStatus::Inactive;
}

UQuestInstance* UQuestManagerSubsystem::GetQuestInstance(FPrimaryAssetId QuestID) const
{
	// if (auto Instance = QuestsInstances.Find(QuestID))
	if (UQuestInstance* Instance = QuestsInstances.FindRef(QuestID))
	{
		return Instance;
	}

	return nullptr;
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
		if (Instance->GetQuestStatus() == Status)
		{
			Result.Add(ID);
		}
	}
	return Result;
}

bool UQuestManagerSubsystem::IsObjectiveCompleted(const FPrimaryAssetId& QuestID, FName ObjectiveID) const
{
	const UQuestInstance* Instance = QuestsInstances.FindRef(QuestID);
	if (!Instance)
	{
		return false;
	}

	for (const UQuestObjective* Objective : Instance->GetObjectives())
	{
		if (Objective && Objective->ObjectiveID == ObjectiveID)
		{
			return Objective->GetStatus() == EQuestObjectiveStatus::Completed;
		}
	}
	return false;
}


// ============================================================================
// Conditions evaluation
// ============================================================================

bool UQuestManagerSubsystem::EvaluateConditions(const TArray<UQuestCondition*>& Conditions) const
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

bool UQuestManagerSubsystem::IsQuestComplete(const UQuestDefinition* Def, const UQuestInstance* Instance) const
{
	if (!Def)
	{
		return false;
	}

	for (int32 i = 0; i < Def->Objectives.Num(); ++i)
	{
		const bool bOptional = Def->Objectives[i] && Def->Objectives[i]->bIsOptional;
		if (!bOptional && Instance->GetObjectives().IsValidIndex(i) && Instance->GetObjectives()[i]->GetStatus() != EQuestObjectiveStatus::Completed)
		{
			return false;
		}
	}
	return true;
}

void UQuestManagerSubsystem::RefreshLockedQuests()
{
	TArray<FPrimaryAssetId> LockedIDs = GetQuestIDsByStatus(EQuestStatus::Inactive);

	for (const FPrimaryAssetId& QuestID : LockedIDs)
	{
		const UQuestDefinition* Def = GetQuestDefinition(QuestID);
		if (!Def)
		{
			continue;
		}

		if (EvaluateUnlockConditions(Def))
		{
			const UQuestInstance* Instance = ActivateQuest(QuestID, nullptr);
			BroadcastQuestStatusChangedMessage(QuestID, Instance->GetQuestStatus());
		}
	}
}

void UQuestManagerSubsystem::RefreshLockedObjectives()
{
	for (auto& [QuestID, Instance] : QuestsInstances)
	{
		if (Instance->GetQuestStatus() != EQuestStatus::Active)
		{
			continue;
		}

		for (UQuestObjective* Objective : Instance->GetObjectives())
		{
			if (Objective && Objective->GetStatus() == EQuestObjectiveStatus::Locked)
			{
				if (EvaluateConditions(Objective->UnlockConditions))
				{
					Objective->ActivateObjective(); // Locked -> Active
				}
			}
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

void UQuestManagerSubsystem::BroadcastQuestStatusChangedMessage(const FPrimaryAssetId& QuestID, const EQuestStatus NewQuestStatus) const
{
	FQuestStatusChangedMessage Msg;
	Msg.QuestID = QuestID;
	Msg.QuestStatus = NewQuestStatus;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage<FQuestStatusChangedMessage>(QuestMessageChannels::TAG_Quest_Status_Changed.GetTag(), Msg);
}

void UQuestManagerSubsystem::OnObjectiveStatusChanged(FGameplayTag Channel, const FQuestObjectiveUpdatedMessage& Message)
{
	UE_LOG(LogQuest, Warning, TEXT("Objective %s Changed for Quest %s"), *Message.ObjectiveID.ToString(), *Message.QuestId.ToString());

	if (Message.Status != EQuestObjectiveStatus::Completed)
	{
		return;
	}

	RefreshLockedObjectives();

	UQuestInstance* Instance = QuestsInstances.FindRef(Message.QuestId);
	if (!Instance || Instance->GetQuestStatus() != EQuestStatus::Active)
	{
		return;
	}

	if (const UQuestDefinition* Def = GetQuestDefinition(Message.QuestId))
	{
		if (IsQuestComplete(Def, Instance))
		{
			CompleteQuest(Message.QuestId);
		}
	}
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

		if (Data.Instances[i]->GetQuestStatus() == EQuestStatus::Active)
		{
			if (const UQuestDefinition* Def = GetQuestDefinition(ID))
			{
				for (int32 ObjIdx = 0; ObjIdx < Def->Objectives.Num(); ++ObjIdx)
				{
					const UQuestObjective* ObjDef = Def->Objectives[ObjIdx];
					const bool bAlreadyDone = Data.Instances[i]->GetObjectives().IsValidIndex(ObjIdx) && Data.Instances[i]->GetObjectives()[ObjIdx]->GetStatus() ==
						EQuestObjectiveStatus::Completed;

					// TODO: Activate quest
				}
			}
		}
	}
}
