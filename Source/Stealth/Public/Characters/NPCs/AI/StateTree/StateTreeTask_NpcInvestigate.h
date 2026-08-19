#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeTask_NpcInvestigate.generated.h"

class ANpcAiController;
class UNpcContextComponent;

USTRUCT()
struct STEALTH_API FStateTreeTask_NpcInvestigateInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ANpcAiController> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UNpcContextComponent> NpcContext = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FVector InvestigateLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float AcceptanceRadius = 75.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float InvestigationDuration = 4.0f;

	UPROPERTY()
	float CurrentInvestigationTime = 0.0f;

	UPROPERTY()
	bool bHasReachedLocation = false;
};

USTRUCT(meta = (DisplayName = "NPC Investigate Location"))
struct STEALTH_API FStateTreeTask_NpcInvestigate : public FStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeTask_NpcInvestigateInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
