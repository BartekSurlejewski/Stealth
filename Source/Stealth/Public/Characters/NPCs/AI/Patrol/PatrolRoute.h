#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "PatrolRoute.generated.h"

/**
 * World Actor representing a designated patrol path or route for a specific location tag.
 * Automatically registers with UPatrolSubsystem.
 */
UCLASS(BlueprintType, Blueprintable)
class STEALTH_API APatrolRoute : public AActor
{
	GENERATED_BODY()

public:
	APatrolRoute();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Location tag that NPCs look for when assigned to patrol this area */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	FGameplayTag LocationTag;

	/** Optional additional tags (e.g. night shift, high security, courtyard) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	FGameplayTagContainer AdditionalTags;

	/** Ordered list of waypoint actors defining the patrol route */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Patrol")
	TArray<TObjectPtr<AActor>> PatrolWaypoints;

	/** Optional spot used when an NPC is performing standing guard duty at this location */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Patrol")
	TObjectPtr<AActor> StandingGuardSpot;

	/** Pattern used to traverse waypoints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	EPatrolMode PatrolMode = EPatrolMode::Loop;

	/** Default wait time in seconds at each waypoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta = (ClampMin = "0.0"))
	float DefaultWaitTime = 2.0f;

	/** Optional per-waypoint wait times override (indexed to match PatrolWaypoints) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	TArray<float> PerWaypointWaitTimes;

	/** Whether this route is currently active and available to NPCs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	bool bIsActive = true;

	/** Get waypoint at index (safe clamp/modulo) */
	UFUNCTION(BlueprintPure, Category = "Patrol")
	AActor* GetWaypoint(int32 Index) const;

	/** Total number of waypoints in the route */
	UFUNCTION(BlueprintPure, Category = "Patrol")
	int32 GetNumWaypoints() const;

	/** Find index of the waypoint closest to the specified location */
	UFUNCTION(BlueprintPure, Category = "Patrol")
	int32 GetClosestWaypointIndex(const FVector& Location) const;

	/** Calculate next waypoint index based on patrol traversal mode */
	UFUNCTION(BlueprintCallable, Category = "Patrol")
	int32 GetNextWaypointIndex(int32 CurrentIndex, UPARAM(ref) bool& bInOutMovingForward) const;

	/** Get wait time for a specific waypoint */
	UFUNCTION(BlueprintPure, Category = "Patrol")
	float GetWaitTimeForWaypoint(int32 Index) const;

	/** Check if route has at least one valid waypoint actor */
	UFUNCTION(BlueprintPure, Category = "Patrol")
	bool HasValidWaypoints() const;

	/** Get the standing guard spot actor if assigned */
	UFUNCTION(BlueprintPure, Category = "Patrol")
	AActor* GetStandingGuardSpot() const;
};
