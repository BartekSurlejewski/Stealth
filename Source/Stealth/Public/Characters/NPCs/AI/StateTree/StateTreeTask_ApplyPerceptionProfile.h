#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "StateTreeTask_ApplyPerceptionProfile.generated.h"

class ANpcAiController;
class UNpcContextComponent;
class UNpcAwarenessComponent;
class UAnimMontage;

USTRUCT()
struct STEALTH_API FStateTreeTask_ApplyPerceptionProfileInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ANpcAiController> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UNpcContextComponent> NpcContext = nullptr;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UNpcAwarenessComponent> AwarenessComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag StateTag;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	ENpcSuspicionMode Mode = ENpcSuspicionMode::Normal;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float SuspicionGainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float SuspicionDecayMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float MaxWalkSpeed = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag StanceTag;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	TObjectPtr<UAnimMontage> EnterStateMontage = nullptr;
};

USTRUCT(meta = (DisplayName = "NPC Apply Perception Profile", Category = "NPC"))
struct STEALTH_API FStateTreeTask_ApplyPerceptionProfile : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeTask_ApplyPerceptionProfileInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
