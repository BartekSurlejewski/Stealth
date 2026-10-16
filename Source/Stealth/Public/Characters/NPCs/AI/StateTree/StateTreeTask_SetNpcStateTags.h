#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "StateTreeTask_SetNpcStateTags.generated.h"

class ANpcAiController;

USTRUCT()
struct STEALTH_API FStateTreeTask_SetNpcStateTagsInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ANpcAiController> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag StateTag;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag StanceTag;
};

USTRUCT(meta = (DisplayName = "Set NPC State Tags", Category = "NPC"))
struct STEALTH_API FStateTreeTask_SetNpcStateTags : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeTask_SetNpcStateTagsInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
