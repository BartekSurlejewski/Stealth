#include "Conditions/QuestCondition_ObjectiveCompleted.h"

#include "QuestManagerSubsystem.h"

bool UQuestCondition_ObjectiveCompleted::IsConditionMet_Implementation(const UObject* WorldContextObject) const
{
	const UQuestManagerSubsystem* QM = UQuestManagerSubsystem::Get(WorldContextObject);
	return QM && QM->IsObjectiveCompleted(QuestID, ObjectiveID);
}

FText UQuestCondition_ObjectiveCompleted::GetDescription_Implementation() const
{
	return FText::Format(NSLOCTEXT("Quest", "ObjCond", "Complete objective: {0}"), FText::FromName(ObjectiveID));
}
