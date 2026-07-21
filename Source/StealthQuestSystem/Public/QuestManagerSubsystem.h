#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/QuestInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestManagerSubsystem.generated.h"

class UAbilitySystemComponent;
struct FQuestEventMessage;
class UQuestCondition;
class UQuestDefinition;
enum class EQuestStatus : uint8;

UCLASS(Config = Game)
class STEALTHQUESTSYSTEM_API UQuestManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	/*Methods*/
public:
	// ------------------------------------------------------------------
	// Subsystem Lifecycle
	// ------------------------------------------------------------------
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category="Quest", meta=(WorldContext="WorldContextObject"))
	static UQuestManagerSubsystem* Get(const UObject* WorldContextObject);

	// ------------------------------------------------------------------
	// Quests Lifecycle
	// ------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void ActivateQuest(FPrimaryAssetId QuestID);
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteQuest(FPrimaryAssetId QuestID);
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void FailQuest(FPrimaryAssetId QuestID);
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void AbandonQuest(FPrimaryAssetId QuestID);
	UFUNCTION(BlueprintCallable, Category = "Quest|Debug")
	void ForceCompleteQuest(FPrimaryAssetId QuestID);
	UFUNCTION(BlueprintCallable, Category = "Quest|Debug")
	void ForceUnlockQuest(FPrimaryAssetId QuestID);

	// ------------------------------------------------------------------
	// Queries (BlueprintPure)
	// ------------------------------------------------------------------
	UFUNCTION(BlueprintPure, Category = "Quest")
	EQuestStatus GetQuestStatus(FPrimaryAssetId QuestID) const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestCompleted(FPrimaryAssetId QuestID) const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestActive(FPrimaryAssetId QuestID) const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestLocked(FPrimaryAssetId QuestID) const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool GetQuestInstance(FPrimaryAssetId QuestID, FQuestInstance& OutInstance) const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FPrimaryAssetId> GetActiveQuestIDs() const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FPrimaryAssetId> GetQuestIDsByStatus(EQuestStatus Status) const;

	// ------------------------------------------------------------------
	// Save / Load
	// ------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Quest|Save")
	void ExportSaveData(FQuestSaveData& OutData) const;
	UFUNCTION(BlueprintCallable, Category = "Quest|Save")
	void ImportSaveData(const FQuestSaveData& Data);
	UPROPERTY(EditDefaultsOnly, Config, Category = "Quest")
	TArray<FPrimaryAssetId> InitialQuestIDs;

private:
	// ------------------------------------------------------------------
	// Helpers
	// ------------------------------------------------------------------
	const UQuestDefinition* GetDefinition(FPrimaryAssetId QuestID) const;

	// ------------------------------------------------------------------
	// Conditions and objectives evaluation
	// ------------------------------------------------------------------

	bool EvaluateFilterConditions(const TArray<UQuestCondition*>& Conditions, const FQuestEventMessage& Msg) const;
	bool EvaluateUnlockConditions(const UQuestDefinition* Def) const;
	bool IsQuestComplete(const UQuestDefinition* Def, const FQuestInstance& Instance) const;
	void RefreshLockedQuests();

	// ------------------------------------------------------------------
	// Player tags
	// ------------------------------------------------------------------

	void GrantTagToPlayer(FGameplayTag Tag);
	bool HasPlayerTag(FGameplayTag Tag) const;

	UAbilitySystemComponent* GetPlayerASC() const;

	// ------------------------------------------------------------------
	// Broadcasts
	// ------------------------------------------------------------------

	void BroadcastQuestStartedMessage(FPrimaryAssetId QuestID) const;
	void BroadcastObjectiveUpdatedMessage(FPrimaryAssetId QuestID, FName ObjectiveID) const;
	void BroadcastQuestCompletedMessage(FPrimaryAssetId QuestID) const;
	void BroadcastQuestFailedMessage(FPrimaryAssetId QuestID) const;
	void BroadcastQuestUnlockedMessage(FPrimaryAssetId QuestID) const;

	/*Properties*/
private:
	// ------------------------------------------------------------------
	// Runtime Data
	// ------------------------------------------------------------------
	UPROPERTY(SaveGame)
	TMap<FPrimaryAssetId, FQuestInstance> QuestsInstances;
	FGameplayTagContainer GrantedQuestTags;
};
