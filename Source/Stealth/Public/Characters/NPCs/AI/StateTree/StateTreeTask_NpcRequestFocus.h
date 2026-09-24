#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "GameplayTagContainer.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "StateTreeTask_NpcRequestFocus.generated.h"

class ANpcAiController;
class UNpcFocusComponent;
class AActor;

USTRUCT()
struct STEALTH_API FStateTreeTask_NpcRequestFocusInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ANpcAiController> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<UNpcFocusComponent> FocusComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bClearFocusOnEnter = false;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag FocusTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	ENpcFocusPriority Priority = ENpcFocusPriority::None;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	TWeakObjectPtr<AActor> FocusActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FVector FocusLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bClearFocusOnExit = false;
};

USTRUCT(meta = (DisplayName = "NPC Request Focus", Category = "NPC"))
struct STEALTH_API FStateTreeTask_NpcRequestFocus : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeTask_NpcRequestFocusInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
