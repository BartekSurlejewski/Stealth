#pragma once

#include "CoreMinimal.h"
#include "Housing/HouseData.h"
#include "AiCrimeEventPayload.generated.h"

class AActor;

/**
 * Structured payload for crime events dispatched across AI systems
 */
USTRUCT(BlueprintType)
struct STEALTH_API FAiCrimeEventPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crime")
	FName HouseId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crime")
	EHouseCrimeType CrimeType = EHouseCrimeType::Trespassing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crime")
	FVector CrimeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crime")
	TWeakObjectPtr<AActor> Perpetrator = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crime")
	TWeakObjectPtr<AActor> VictimOrTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crime")
	bool bIsPrimaryInvestigator = false;
};
