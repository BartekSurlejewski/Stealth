#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeTask_PlayEnterMontage.generated.h"

class ANpcAiController;
class UAnimMontage;

USTRUCT()
struct STEALTH_API FStateTreeTask_PlayEnterMontageInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ANpcAiController> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<UAnimMontage> EnterStateMontage = nullptr;
};

USTRUCT(meta = (DisplayName = "Play Enter Montage", Category = "NPC"))
struct STEALTH_API FStateTreeTask_PlayEnterMontage : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeTask_PlayEnterMontageInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
