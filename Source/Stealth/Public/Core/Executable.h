#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Executable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UExecutable : public UInterface
{
	GENERATED_BODY()
};

// Represents simple actions that can be executed during gameplay. For example, starting a quest when chosing a dialogue option. 
// Treat is as a way to hook custom behaviours into gameplay loop.
class STEALTH_API IExecutable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Actions")
	void PerformAction(const UObject* WorldContextObject);

	virtual void PerformAction_Implementation(const UObject* WorldContextObject) {}
};
