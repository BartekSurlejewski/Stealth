#include "Conditions/QuestCondition_And.h"

bool UQuestCondition_And::IsConditionMet_Implementation(const UObject* WCO) const
{
	for (const auto* Cond : SubConditions)
	{
		if (!Cond->IsConditionMet(WCO)) return false;
	}

	return true;
}
