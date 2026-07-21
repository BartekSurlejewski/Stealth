#include "QuestDefinitionFactory.h"

#include "Data/QuestDefinition.h"

UQuestDefinitionFactory::UQuestDefinitionFactory()
{
	SupportedClass = UQuestDefinition::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UQuestDefinitionFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UQuestDefinition* NewQuest = NewObject<UQuestDefinition>(InParent, Class, Name, Flags);
	// Set default quest values here
	return NewQuest;
}
