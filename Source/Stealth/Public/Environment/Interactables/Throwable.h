#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Throwable.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UThrowable : public UInterface
{
	GENERATED_BODY()
};


class STEALTH_API IThrowable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Throw(FVector ThrowDirection);
};
