#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StealthMessages.generated.h"

USTRUCT(BlueprintType)
struct FBooleanMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	bool bValue = false;
};

USTRUCT(BlueprintType)
struct FInteractableMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	AActor* InteractableActor;
};
