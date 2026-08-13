#include "UI/JournalMenu/JournalMenu.h"

#include "QuestManagerSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Data/QuestDefinition.h"

void UJournalMenu::OnShow_Implementation()
{
	Super::OnShow_Implementation();

	const UQuestManagerSubsystem* QuestManager = UQuestManagerSubsystem::Get(this);
	if (!QuestManager)
	{
		return;
	}

	QuestDescriptionsContainer->ClearChildren();

	for (const FPrimaryAssetId& QuestID : QuestManager->GetActiveQuestIDs())
	{
		UTextBlock* QuestTitleBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		// TObjectPtr<UTextBlock> QuestTitleBlock = NewObject<UTextBlock>(this);
		QuestDescriptionsContainer->AddChild(QuestTitleBlock);
		QuestTitleBlock->SetText(QuestManager->GetQuestDefinition(QuestID)->QuestTitle);
	}
}
