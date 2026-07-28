#include "Objectives/QuestObjective.h"
#include "Conditions/QuestCondition.h"
#include "Data/QuestInstance.h"

void UQuestObjective::ActivateObjective_Implementation()
{
	if (AreAllConditionsMet())
	{
		SetStatus(EQuestObjectiveStatus::Completed);
		return;
	}

	for (UQuestCondition* Condition : ObjectiveConditions)
	{
		Condition->OnQuestConditionStatusChanged.AddDynamic(this, &UQuestObjective::QuestCondition_StatusChanged);
		Condition->Activate();
	}

	SetStatus(EQuestObjectiveStatus::Active);
}

void UQuestObjective::DeactivateObjective_Implementation()
{
	for (UQuestCondition* Condition : ObjectiveConditions)
	{
		Condition->OnQuestConditionStatusChanged.RemoveAll(this);
		Condition->Deactivate();
		ActivateObjective();
	}

	SetStatus(EQuestObjectiveStatus::Locked);
}

void UQuestObjective::ForceCompleteObjective()
{
	for (const UQuestCondition* Condition : ObjectiveConditions)
	{
		if (!Condition)
		{
			continue;
		}

		Condition->ForceComplete(this);
	}
}

UQuestInstance* UQuestObjective::GetParentQuestInstance()
{
	if (!CachedParentQuest)
	{
		CachedParentQuest = Cast<UQuestInstance>(GetOuter());
	}
	return CachedParentQuest;
}

AActor* UQuestObjective::GetQuestInstigator()
{
	UQuestInstance* ParentQuest = GetParentQuestInstance();
	return ParentQuest ? ParentQuest->GetInstigator() : nullptr;
}

AActor* UQuestObjective::GetQuestContextActor(FName Key)
{
	UQuestInstance* ParentQuest = GetParentQuestInstance();
	return ParentQuest ? ParentQuest->GetContextActor(Key) : nullptr;
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

bool UQuestObjective::AreAllConditionsMet() const
{
	bool bConditionsMet = true;
	for (UQuestCondition* Condition : ObjectiveConditions)
	{
		if (!Condition)
		{
			continue;
		}

		if (!Condition->IsConditionMet(this))
		{
			bConditionsMet = false;
			break;
		}
	}

	return bConditionsMet;
}

void UQuestObjective::QuestCondition_StatusChanged(UQuestCondition* Condition, bool bIsConditionMet)
{
	if (bIsConditionMet && AreAllConditionsMet())
	{
		SetStatus(EQuestObjectiveStatus::Completed);
	}
}
