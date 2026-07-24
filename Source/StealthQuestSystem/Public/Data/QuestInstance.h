#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UObject/Object.h"
#include "QuestInstance.generated.h"

enum class EQuestObjectiveStatus : uint8;
class UQuestObjective;

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
	Inactive,
	Active,
	Finished,
};

UCLASS(Blueprintable)
class STEALTHQUESTSYSTEM_API UQuestInstance : public UObject
{
	GENERATED_BODY()
	/*Methods*/
public:
	void Initialize(const FPrimaryAssetId& NewQuestDefinitionID, const EQuestStatus& NewQuestStatus, TArray<TObjectPtr<UQuestObjective>>&& NewObjectives);

	UFUNCTION()
	void ActivateQuest();
	UFUNCTION()
	void DeactivateQuest();

	// Getters & Setters
	[[nodiscard]] const TArray<TObjectPtr<UQuestObjective>>& GetObjectives() const { return Objectives; }
	[[nodiscard]] const FPrimaryAssetId& GetQuestDefinitionID() const { return QuestDefinitionID; }
	[[nodiscard]] EQuestStatus GetQuestStatus() const { return Status; }

	void SetStatus(EQuestStatus NewStatus);

protected:
	UFUNCTION()
	void ActivateObjective(UQuestObjective* NewObjective);
	UFUNCTION()
	void DeactivateObjective(UQuestObjective* Objective) const;

	UFUNCTION()
	void OnObjectiveStatusChanged(FName ObjectiveId, EQuestObjectiveStatus NewStatus);

	/*Properties*/
protected:
	UPROPERTY(BlueprintReadOnly, Category="Quest|Instance")
	FPrimaryAssetId QuestDefinitionID;
	UPROPERTY(BlueprintReadOnly, Category="Quest|Instance")
	EQuestStatus Status = EQuestStatus::Inactive;
	UPROPERTY(BlueprintReadOnly, Category="Quest|Instance")
	TArray<TObjectPtr<UQuestObjective>> Objectives;
};

USTRUCT(BlueprintType)
struct FQuestSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPrimaryAssetId> QuestIDs;
	UPROPERTY()
	TArray<TObjectPtr<UQuestInstance>> Instances;
	UPROPERTY()
	FGameplayTagContainer GrantedQuestTags; // Tags granted by quests system
};
