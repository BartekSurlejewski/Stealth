#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeTask_SetNpcWalkSpeed.generated.h"

class ANpcAiController;

USTRUCT()
struct STEALTH_API FStateTreeTask_SetNpcWalkSpeedInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ANpcAiController> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MaxWalkSpeed = 200.0f;
};

USTRUCT(meta = (DisplayName = "Set NPC Walk Speed", Category = "NPC"))
struct STEALTH_API FStateTreeTask_SetNpcWalkSpeed : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeTask_SetNpcWalkSpeedInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
