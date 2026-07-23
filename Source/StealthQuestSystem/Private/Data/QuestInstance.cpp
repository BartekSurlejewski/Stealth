#include "Data/QuestInstance.h"

#include "Conditions/QuestCondition.h"
#include "Data/QuestSystemMessages.h"
#include "Objectives/QuestObjective.h"


void UQuestInstance::Initialize(const FPrimaryAssetId& NewQuestDefinitionID, const EQuestStatus& NewQuestStatus, TArray<TObjectPtr<UQuestObjective>>&& NewObjectives)
{
	this->QuestDefinitionID = NewQuestDefinitionID;
	this->Status = NewQuestStatus;
	this->Objectives = MoveTemp(NewObjectives);
}

void UQuestInstance::ActivateQuest()
{
	for (const TObjectPtr<UQuestObjective> Objective : Objectives)
	{
		bool bUnlockConditionsMet = true;
		for (const UQuestCondition* Cond : Objective->UnlockConditions)
		{
			if (Cond && !Cond->IsConditionMet(this))
			{
				bUnlockConditionsMet = false;
			}
		}
		if (bUnlockConditionsMet)
		{
			Objective->ActivateObjective();
		}
	}

	SetStatus(EQuestStatus::Active);
}

void UQuestInstance::DeactivateQuest()
{
	for (const TObjectPtr<UQuestObjective> Objective : Objectives)
	{
		Objective->DeactivateObjective();
	}

	SetStatus(EQuestStatus::Inactive);
}

void UQuestInstance::SetStatus(EQuestStatus NewStatus)
{
	if (Status == NewStatus)
	{
		return;
	}

	Status = NewStatus;
}

void UQuestInstance::ActivateObjective(UQuestObjective* NewObjective)
{
	if (!NewObjective)
	{
		return;
	}

	NewObjective->OnQuestObjectiveStatusChanged.AddDynamic(this, &UQuestInstance::OnObjectiveStatusChanged);
	NewObjective->ActivateObjective();
}

void UQuestInstance::DeactivateObjective(UQuestObjective* Objective) const
{
	if (!Objective)
	{
		return;
	}

	Objective->OnQuestObjectiveStatusChanged.RemoveAll(this);
	Objective->DeactivateObjective();
}

void UQuestInstance::OnObjectiveStatusChanged(FName ObjectiveId, EQuestObjectiveStatus NewStatus)
{
	FQuestObjectiveUpdatedMessage Msg;
	Msg.QuestId = QuestDefinitionID;
	Msg.ObjectiveID = ObjectiveId;
	Msg.Status = NewStatus;
	UGameplayMessageSubsystem::Get(this).BroadcastMessage<FQuestObjectiveUpdatedMessage>(QuestMessageChannels::TAG_Quest_Objective_Status_Changed.GetTag(), Msg);
}
