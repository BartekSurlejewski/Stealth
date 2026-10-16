#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "StateTreeTask_ApplyPerceptionTuning.generated.h"

class ANpcAiController;

USTRUCT()
struct STEALTH_API FStateTreeTask_ApplyPerceptionTuningInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ANpcAiController> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FNpcPerceptionTuningParams TuningParams;
};

USTRUCT(meta = (DisplayName = "Apply Perception Tuning", Category = "NPC"))
struct STEALTH_API FStateTreeTask_ApplyPerceptionTuning : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeTask_ApplyPerceptionTuningInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
