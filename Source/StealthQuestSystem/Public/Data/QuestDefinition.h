#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "QuestDefinition.generated.h"

class UQuestReward;
class UQuestCondition;
class UQuestObjective;

UCLASS(BlueprintType)
class STEALTHQUESTSYSTEM_API UQuestDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("QuestDefinition"), GetFName());
	}

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Presentation")
	FText QuestTitle;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Presentation", meta=(MultiLine=true))
	FText QuestDescription;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Presentation")
	FGameplayTag QuestCategory;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category="Structure")
	bool bIsSequential = true;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Structure")
	TArray<TObjectPtr<UQuestObjective>> Objectives;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Unlock")
	TArray<UQuestCondition*> UnlockConditions;

	UPROPERTY(EditDefaultsOnly, Instanced, Category="Rewards")
	TArray<UQuestReward*> Rewards;

	UPROPERTY(EditDefaultsOnly, Category="Metadata")
	bool bIsRepeatable = false;

	UPROPERTY(EditDefaultsOnly, Category="Metadata", meta=(EditCondition="bIsRepeatable"))
	float RepeatCooldownHours = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Metadata")
	int32 SortPriority = 0; // Order in a journal
};
