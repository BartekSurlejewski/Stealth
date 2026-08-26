#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Destructible.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UDestructible : public UInterface
{
	GENERATED_BODY()
};

class STEALTH_API IDestructible
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Destructible")
	void Destroy();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Destructible")
	void Fix();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Destructible")
	bool WasDestroyed();
};
