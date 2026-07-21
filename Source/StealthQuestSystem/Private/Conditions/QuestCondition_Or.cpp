#include "Conditions/QuestCondition_Or.h"

bool UQuestCondition_Or::IsConditionMet_Implementation(const UObject* WCO) const
{
	for (const auto* Cond : SubConditions)
	{
		if (Cond->IsConditionMet(WCO)) return true;
	}

	return false;
}
