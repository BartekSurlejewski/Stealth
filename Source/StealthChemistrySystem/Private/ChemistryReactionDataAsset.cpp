#include "ChemistryReactionDataAsset.h"
#include "Engine/DataTable.h"

void UChemistryReactionDataAsset::GetAllRules(TArray<FChemistryReactionRule>& OutRules) const
{
	OutRules = ReactionRules;

	if (IsValid(ReactionDataTable))
	{
		ReactionDataTable->ForeachRow<FChemistryReactionRule>(
			TEXT("UChemistryReactionDataAsset::GetAllRules"),
			[&OutRules](const FName& RowName, const FChemistryReactionRule& Row)
			{
				OutRules.Add(Row);
			});
	}
}
