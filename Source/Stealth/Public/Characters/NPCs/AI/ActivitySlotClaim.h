#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ActivitySlotClaim.generated.h"

class AActor;

/**
 * Information about an active activity slot / smart object
 */
USTRUCT(BlueprintType)
struct STEALTH_API FActivitySlotClaim
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "AI|Activity")
	FGuid ClaimerGuid;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Activity")
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Activity")
	FGameplayTag ActivityTag;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Activity")
	float ClaimTimestamp = 0.0f;
};
