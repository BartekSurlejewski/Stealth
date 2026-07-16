#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StealthMessages.generated.h"

class AActor;
enum class ETimeOfDay : uint8;
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

USTRUCT(BlueprintType)
struct FTimeChangedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 NewHour;
	UPROPERTY(BlueprintReadWrite, Category="Message")
	int32 NewMinute;
};

USTRUCT(BlueprintType)
struct FTimeOfDayChangedMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	ETimeOfDay NewTimeOfDay;
};

USTRUCT(BlueprintType)
struct FGameplayTagMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	FGameplayTag GameplayTag;
};

USTRUCT(BlueprintType)
struct FInputMessage
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, Category="Message")
	FGameplayTag GameplayTag;
};
