#pragma once

#pragma once

#include "CoreMinimal.h"
#include "HouseData.generated.h"

class AActor;
class ATriggerVolume;
class ANpcCharacter;

/** Types of crimes that can occur in or around a house */
UENUM(BlueprintType)
enum class EHouseCrimeType : uint8
{
	None,
	Trespassing,
	LockpickingDoor,
	LockpickingContainer,
	Theft,
	Assault
};

/** Access relationship of an actor to a house */
UENUM(BlueprintType)
enum class EHouseAccessLevel : uint8
{
	Forbidden, // Intrusion / Trespassing if entered
	VisitorAllowed, // Public during day or allowed guest
	Resident, // Co-owner / Roommate
	Owner // Primary owner
};

/** Routine and interaction markers within a house */
USTRUCT(BlueprintType)
struct FHouseActivityPoints
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing|Markers")
	TMap<FGuid, TObjectPtr<AActor>> AssignedBeds; // NpcGUID -> Bed Actor / Smart Object

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing|Markers")
	TArray<TObjectPtr<AActor>> LeisureSpots; // Chairs, tables, fireplaces

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Housing|Markers")
	TArray<TObjectPtr<AActor>> WorkSpots; // Crafting tables, counters
};

/** Crime event payload sent to AI controllers / Crime systems */
USTRUCT(BlueprintType)
struct FHouseCrimeReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Housing|Crime")
	FName HouseId;

	UPROPERTY(BlueprintReadOnly, Category = "Housing|Crime")
	EHouseCrimeType CrimeType = EHouseCrimeType::None;

	UPROPERTY(BlueprintReadOnly, Category = "Housing|Crime")
	TObjectPtr<AActor> Perpetrator = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Housing|Crime")
	TObjectPtr<AActor> VictimOrTarget = nullptr; // e.g. the chest looted or door picked

	UPROPERTY(BlueprintReadOnly, Category = "Housing|Crime")
	FVector CrimeLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Housing|Crime")
	float Timestamp = 0.0f;
};
