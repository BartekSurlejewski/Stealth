#include "Objectives/QuestObjective.h"
#include "Conditions/QuestCondition.h"

void UQuestObjective::ActivateObjective_Implementation()
{
	for (UQuestCondition* Condition : ObjectiveConditions)
	{
		if (!Condition)
		{
			continue;
		}

		if (Condition->IsConditionMet(this))
		{
			SetStatus(EQuestObjectiveStatus::Completed);
			return;
		}
	}

	SetStatus(EQuestObjectiveStatus::Active);
}

void UQuestObjective::DeactivateObjective_Implementation()
{
	SetStatus(EQuestObjectiveStatus::Active);
}

void UQuestObjective::ForceCompleteObjective()
{
	for (const UQuestCondition* Condition : ObjectiveConditions)
	{
		if (!Condition)
		{
			continue;
		}

		Condition->ForceComplete();
	}

	SetStatus(EQuestObjectiveStatus::Completed);
}

void UQuestObjective::SetStatus(EQuestObjectiveStatus NewStatus)
{
	if (Status == NewStatus)
	{
		return;
	}

	Status = NewStatus;
	OnQuestObjectiveStatusChanged.Broadcast(ObjectiveID, Status);
}
