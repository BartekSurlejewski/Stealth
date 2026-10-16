#pragma once

#include "CoreMinimal.h"
#include "StateTreeEvaluatorBase.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "StateTreePerceptionEvaluator.generated.h"

class ANpcAiController;

USTRUCT()
struct STEALTH_API FStateTreePerceptionEvaluatorInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ANpcAiController> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Output")
	FNpcAwarenessState AwarenessState;

	UPROPERTY(EditAnywhere, Category = "Output")
	FNpcFocusTarget CurrentFocus;

	UPROPERTY(EditAnywhere, Category = "Output")
	float SuspicionThreshold_Suspicious = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Output")
	float SuspicionThreshold_Alert = 75.0f;
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
