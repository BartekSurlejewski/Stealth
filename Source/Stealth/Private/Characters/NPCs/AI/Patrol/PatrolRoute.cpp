#include "Characters/NPCs/AI/Patrol/PatrolRoute.h"
#include "Characters/NPCs/AI/Patrol/PatrolSubsystem.h"
#include "Characters/NPCs/AI/ActivityPointSubsystem.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "Engine/World.h"

APatrolRoute::APatrolRoute()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

void APatrolRoute::BeginPlay()
{
	Super::BeginPlay();

	if (UPatrolSubsystem* Subsystem = UPatrolSubsystem::Get(this))
	{
		Subsystem->RegisterPatrolRoute(this);
	}

	if (StandingGuardSpot && LocationTag.IsValid())
	{
		if (UActivityPointSubsystem* ActivitySubsystem = UActivityPointSubsystem::Get(this))
		{
			FGameplayTagContainer ActivityTags;
			ActivityTags.AddTag(StealthAiTags::TAG_NPC_Activity_GuardPost);
			ActivityTags.AddTag(LocationTag);
			ActivitySubsystem->RegisterActivityPoint(StandingGuardSpot.Get(), ActivityTags);
		}
	}
}

void APatrolRoute::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (StandingGuardSpot)
	{
		if (UActivityPointSubsystem* ActivitySubsystem = UActivityPointSubsystem::Get(this))
		{
			ActivitySubsystem->UnregisterActivityPoint(StandingGuardSpot.Get());
		}
	}

	if (UPatrolSubsystem* Subsystem = UPatrolSubsystem::Get(this))
	{
		Subsystem->UnregisterPatrolRoute(this);
	}

	Super::EndPlay(EndPlayReason);
}

AActor* APatrolRoute::GetWaypoint(int32 Index) const
{
	if (PatrolWaypoints.IsEmpty())
	{
		return nullptr;
	}

	if (PatrolWaypoints.IsValidIndex(Index))
	{
		return PatrolWaypoints[Index].Get();
	}

	const int32 SafeIndex = FMath::Abs(Index) % PatrolWaypoints.Num();
	return PatrolWaypoints[SafeIndex].Get();
}

int32 APatrolRoute::GetNumWaypoints() const
{
	return PatrolWaypoints.Num();
}

int32 APatrolRoute::GetClosestWaypointIndex(const FVector& Location) const
{
	if (PatrolWaypoints.IsEmpty())
	{
		return 0;
	}

	int32 ClosestIndex = 0;
	float MinDistSq = TNumericLimits<float>::Max();

	for (int32 i = 0; i < PatrolWaypoints.Num(); ++i)
	{
		if (const AActor* Waypoint = PatrolWaypoints[i].Get())
		{
			const float DistSq = FVector::DistSquared(Location, Waypoint->GetActorLocation());
			if (DistSq < MinDistSq)
			{
				MinDistSq = DistSq;
				ClosestIndex = i;
			}
		}
	}

	return ClosestIndex;
}

int32 APatrolRoute::GetNextWaypointIndex(int32 CurrentIndex, bool& bInOutMovingForward) const
{
	const int32 Count = PatrolWaypoints.Num();
	if (Count <= 1)
	{
		return 0;
	}

	switch (PatrolMode)
	{
	case EPatrolMode::Loop:
		return (CurrentIndex + 1) % Count;

	case EPatrolMode::PingPong:
		if (bInOutMovingForward)
		{
			if (CurrentIndex + 1 >= Count)
			{
				bInOutMovingForward = false;
				return FMath::Max(0, CurrentIndex - 1);
			}
			return CurrentIndex + 1;
		}
		else
		{
			if (CurrentIndex - 1 < 0)
			{
				bInOutMovingForward = true;
				return FMath::Min(Count - 1, CurrentIndex + 1);
			}
			return CurrentIndex - 1;
		}

	case EPatrolMode::Once:
		return FMath::Min(Count - 1, CurrentIndex + 1);

	case EPatrolMode::Random:
		if (Count <= 1)
		{
			return 0;
		}
		{
			int32 NextRandom = FMath::RandRange(0, Count - 1);
			if (NextRandom == CurrentIndex && Count > 1)
			{
				NextRandom = (CurrentIndex + 1) % Count;
			}
			return NextRandom;
		}

	default:
		return (CurrentIndex + 1) % Count;
	}
}

float APatrolRoute::GetWaitTimeForWaypoint(int32 Index) const
{
	if (PerWaypointWaitTimes.IsValidIndex(Index))
	{
		return PerWaypointWaitTimes[Index];
	}
	return DefaultWaitTime;
}

bool APatrolRoute::HasValidWaypoints() const
{
	for (const TObjectPtr<AActor>& Waypoint : PatrolWaypoints)
	{
		if (Waypoint.Get() != nullptr)
		{
			return true;
		}
	}
	return false;
}

AActor* APatrolRoute::GetStandingGuardSpot() const
{
	return StandingGuardSpot.Get();
}
