#pragma once

#include "CoreMinimal.h"
#include "Core/Executable.h"
#include "UObject/Object.h"
#include "USpecialDialogueBehaviour.generated.h"

UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable)
class STEALTH_API USpecialDialogueBehaviour : public UObject, public IExecutable
{
	GENERATED_BODY()

public:
	virtual void PerformAction_Implementation(const UObject* WorldContextObject) override {};
};
