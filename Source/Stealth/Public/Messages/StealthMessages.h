#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "StealthMessages.generated.h"

class UDailyRegimenTask;

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

USTRUCT(BlueprintType)
struct FDailyTaskStartedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	UDailyRegimenTask* StartedTask;
};

USTRUCT(BlueprintType)
struct FDailyTaskEndedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	UDailyRegimenTask* EndedTask;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	bool bPlayerSucceeded;
};
