#pragma once

#include "CoreMinimal.h"
#include "StateTreeConditionBase.h"
#include "GameplayTagContainer.h"
#include "StateTreeCondition_NpcFocus.generated.h"

class ANpcAiController;

USTRUCT()
struct STEALTH_API FStateTreeCondition_NpcFocusedOnPlayerInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ANpcAiController> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};

/** Condition checking if the NPC is currently focused on any player */
USTRUCT(meta = (DisplayName = "Focused On Player", Category = "NPC"))
struct STEALTH_API FStateTreeCondition_NpcFocusedOnPlayer : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeCondition_NpcFocusedOnPlayerInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};

USTRUCT()
struct STEALTH_API FStateTreeCondition_NpcFocusMatchesTagInstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Context")
	TObjectPtr<ANpcAiController> Controller = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	FGameplayTag RequiredFocusTag;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bExactMatch = false;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bInvert = false;
};

/** Condition checking if the NPC current focus matches a specific tag */
USTRUCT(meta = (DisplayName = "Focus Matches Tag", Category = "NPC"))
struct STEALTH_API FStateTreeCondition_NpcFocusMatchesTag : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	using FInstanceDataType = FStateTreeCondition_NpcFocusMatchesTagInstanceData;

	virtual const UStruct* GetInstanceDataType() const override
	{
		return FInstanceDataType::StaticStruct();
	}

	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;
};
