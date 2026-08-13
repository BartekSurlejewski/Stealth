#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Data/QuestInstance.h"
#include "Data/QuestSystemMessages.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestManagerSubsystem.generated.h"

class UAbilitySystemComponent;
struct FQuestEventMessage;
class UQuestCondition;
class UQuestDefinition;
enum class EQuestStatus : uint8;

UCLASS()
class STEALTHQUESTSYSTEM_API UQuestManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	/*Methods*/
public:
	// ------------------------------------------------------------------
	// Subsystem Lifecycle
	// ------------------------------------------------------------------
	UFUNCTION()
	virtual void PlayerInitialize();
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="Quest", meta=(WorldContext="WorldContextObject"))
	static UQuestManagerSubsystem* Get(const UObject* WorldContextObject);

	// ------------------------------------------------------------------
	// Quests Lifecycle
	// ------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Quest")
	UQuestInstance* ActivateQuest(FPrimaryAssetId QuestID, AActor* Instigator);
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
	const TArray<UQuestInstance*> GetQuestsInstances() const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	EQuestStatus GetQuestStatus(FPrimaryAssetId QuestID) const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestCompleted(FPrimaryAssetId QuestID) const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestActive(FPrimaryAssetId QuestID) const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsQuestLocked(FPrimaryAssetId QuestID) const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	UQuestInstance* GetQuestInstance(FPrimaryAssetId QuestID) const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FPrimaryAssetId> GetActiveQuestIDs() const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	TArray<FPrimaryAssetId> GetQuestIDsByStatus(EQuestStatus Status) const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	bool IsObjectiveCompleted(const FPrimaryAssetId& QuestID, FName ObjectiveID) const;
	UFUNCTION(BlueprintPure, Category = "Quest")
	const UQuestDefinition* GetQuestDefinition(const FPrimaryAssetId& QuestID) const;

	// ------------------------------------------------------------------
	// Save / Load
	// ------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Quest|Save")
	void ExportSaveData(FQuestSaveData& OutData) const;
	UFUNCTION(BlueprintCallable, Category = "Quest|Save")
	void ImportSaveData(const FQuestSaveData& Data);

private:
	// ------------------------------------------------------------------
	// Conditions and objectives evaluation
	// ------------------------------------------------------------------

	bool EvaluateConditions(const TArray<UQuestCondition*>& Conditions) const;
	bool EvaluateUnlockConditions(const UQuestDefinition* Def) const;
	bool IsQuestComplete(const UQuestDefinition* Def, const UQuestInstance* Instance) const;
	void RefreshLockedQuests();
	void RefreshLockedObjectives();

	// ------------------------------------------------------------------
	// Player tags
	// ------------------------------------------------------------------

	void GrantTagToPlayer(FGameplayTag Tag);
	bool HasPlayerTag(FGameplayTag Tag) const;

	UAbilitySystemComponent* GetPlayerASC() const;

	// ------------------------------------------------------------------
	// Broadcasts
	// ------------------------------------------------------------------

	void BroadcastQuestStatusChangedMessage(const FPrimaryAssetId& QuestID, EQuestStatus NewQuestStatus) const;

	void OnObjectiveStatusChanged(FGameplayTag Channel, const FQuestObjectiveUpdatedMessage& Message);

	/*Properties*/

private:
	// ------------------------------------------------------------------
	// Runtime Data
	// ------------------------------------------------------------------
	UPROPERTY(SaveGame)
	TMap<FPrimaryAssetId, TObjectPtr<UQuestInstance>> QuestsInstances;
	UPROPERTY()
	FGameplayTagContainer GrantedQuestTags;

	// ------------------------------------------------------------------
	// Listener handles
	// ------------------------------------------------------------------
	FGameplayMessageListenerHandle QuestObjectiveStatusChangedHandle;
};
