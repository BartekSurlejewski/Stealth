#include "Characters/NPCs/AI/StateTree/StateTreeCondition_NpcFocus.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "StateTreeExecutionContext.h"

bool FStateTreeCondition_NpcFocusedOnPlayer::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const ANpcAiController* Controller = InstanceData.Controller;
	if (!Controller)
	{
		return InstanceData.bInvert;
	}

	const UNpcFocusComponent* FocusComp = Controller->GetFocusComponent();
	const bool bFocusedOnPlayer = FocusComp ? FocusComp->IsFocusedOnPlayer() : false;

	return InstanceData.bInvert ? !bFocusedOnPlayer : bFocusedOnPlayer;
}

bool FStateTreeCondition_NpcFocusMatchesTag::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
	const ANpcAiController* Controller = InstanceData.Controller;
	if (!Controller)
	{
		return InstanceData.bInvert;
	}

	const UNpcFocusComponent* FocusComp = Controller->GetFocusComponent();
	if (!FocusComp)
	{
		return InstanceData.bInvert;
	}

	const FGameplayTag CurrentTag = FocusComp->GetCurrentFocus().FocusTag;
	bool bMatches = false;

	if (InstanceData.bExactMatch)
	{
		bMatches = CurrentTag.MatchesTagExact(InstanceData.RequiredFocusTag);
	}
	else
	{
		bMatches = CurrentTag.MatchesTag(InstanceData.RequiredFocusTag);
	}

	return InstanceData.bInvert ? !bMatches : bMatches;
}
