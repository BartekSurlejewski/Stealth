#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "UObject/Object.h"
#include "QuestDefinitionFactory.generated.h"


UCLASS()
class STEALTHQUESTSYSTEMEDITOR_API UQuestDefinitionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UQuestDefinitionFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
