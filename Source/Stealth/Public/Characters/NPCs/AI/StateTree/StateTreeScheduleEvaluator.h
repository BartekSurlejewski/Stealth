#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "GameplayTagContainer.h"
#include "Characters/NPCs/AI/Schedule/NpcScheduleAsset.h"
#include "StateTreeScheduleEvaluator.generated.h"

class UNpcScheduleComponent;

USTRUCT()
struct STEALTH_API FStateTreeScheduleEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UNpcScheduleComponent> ScheduleComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTag ActiveActivityTag;

	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTag ActiveLocationTag;

	UPROPERTY(EditAnywhere, Category = "Output")
	TWeakObjectPtr<AActor> TargetActivityPoint = nullptr;

	UPROPERTY(EditAnywhere, Category = "Output")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bCanBeInterrupted = true;

	UPROPERTY(EditAnywhere, Category = "Output")
	int32 Priority = 0;
};

USTRUCT(meta = (DisplayName = "NPC Schedule Evaluator", Category = "NPC"))
struct STEALTH_API FStateTreeScheduleEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeScheduleEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
