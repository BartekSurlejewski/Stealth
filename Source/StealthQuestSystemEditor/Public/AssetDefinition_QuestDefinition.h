#pragma once

#include "CoreMinimal.h"
#include "AssetDefinitionDefault.h"
#include "Data/QuestDefinition.h"
#include "Misc/AssetCategoryPath.h"
#include "AssetDefinition_QuestDefinition.generated.h"

UCLASS()
class STEALTHQUESTSYSTEMEDITOR_API UAssetDefinition_QuestDefinition : public UAssetDefinitionDefault
{
	GENERATED_BODY()

public:
	virtual FText GetAssetDisplayName() const override { return NSLOCTEXT("Quest", "Name", "Quest definition"); }
	virtual FLinearColor GetAssetColor() const override { return FLinearColor(0.9f, 0.6f, 0.1f); }
	virtual TSoftClassPtr<UObject> GetAssetClass() const override { return UQuestDefinition::StaticClass(); }

	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override
	{
		static const FAssetCategoryPath Categories[] = {FAssetCategoryPath(NSLOCTEXT("Quest", "Cat", "Quest System"))};
		return Categories;
	}
};
