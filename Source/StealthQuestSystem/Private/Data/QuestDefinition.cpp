#include "Data/QuestDefinition.h"

#include "Misc/DataValidation.h"
#include "Objectives/QuestObjective.h"

#if WITH_EDITOR
EDataValidationResult UQuestDefinition::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (QuestTitle.IsEmpty())
	{
		Context.AddError(NSLOCTEXT("Quest", "NoTitle", "Quest doesn't have a title."));
		Result = EDataValidationResult::Invalid;
	}

	if (Objectives.IsEmpty())
	{
		Context.AddError(NSLOCTEXT("Quest", "NoObjectives", "Quest doesn't have any objective."));
		Result = EDataValidationResult::Invalid;
	}

	// ObjectiveID duplicates
	TSet<FName> SeenIDs;
	for (const UQuestObjective* Obj : Objectives)
	{
		if (!Obj)
		{
			Context.AddError(NSLOCTEXT("Quest", "NullObjective", "Empty slot in objectives list."));
			Result = EDataValidationResult::Invalid;
			continue;
		}
		if (SeenIDs.Contains(Obj->ObjectiveID))
		{
			Context.AddError(FText::Format(
				NSLOCTEXT("Quest", "DupeID", "Objective ID duplicate: {0}"),
				FText::FromName(Obj->ObjectiveID)));
			Result = EDataValidationResult::Invalid;
		}
		SeenIDs.Add(Obj->ObjectiveID);
	}

	return Result;
}
#endif
