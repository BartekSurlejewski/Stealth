#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "StateTreePerceptionEvaluator.generated.h"

class UNpcContextComponent;

USTRUCT()
struct STEALTH_API FStateTreePerceptionEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UNpcContextComponent> NpcContext = nullptr;

	UPROPERTY(EditAnywhere, Category = "Output")
	FGameplayTag CurrentStateTag;

	UPROPERTY(EditAnywhere, Category = "Output")
	float Awareness = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	ENpcAlertLevel AlertLevel = ENpcAlertLevel::Unaware;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bHasPlayerLineOfSight = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bEffectivelySeesPlayer = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	bool bIsPlayerPerformingIllegalAction = false;

	UPROPERTY(EditAnywhere, Category = "Output")
	FVector LastKnownPlayerPos = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Output")
	FVector LastHeardSoundLocation = FVector::ZeroVector;
};

USTRUCT(meta = (DisplayName = "NPC Perception Evaluator", Category = "NPC"))
struct STEALTH_API FStateTreePerceptionEvaluator : public FStateTreeEvaluatorCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreePerceptionEvaluatorInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual void TreeStart(FStateTreeExecutionContext& Context) const override;
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
};
