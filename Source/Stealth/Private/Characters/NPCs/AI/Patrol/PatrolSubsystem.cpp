#include "Characters/NPCs/AI/Patrol/PatrolSubsystem.h"
#include "Characters/NPCs/AI/Patrol/PatrolRoute.h"
#include "EngineUtils.h"

void UPatrolSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Collect any patrol routes already present in the world at startup
	for (TActorIterator<APatrolRoute> It(&InWorld); It; ++It)
	{
		RegisterPatrolRoute(*It);
	}
}

void UPatrolSubsystem::RegisterPatrolRoute(APatrolRoute* Route)
{
	if (!Route)
	{
		return;
	}

	for (const TWeakObjectPtr<APatrolRoute>& ExistingRoute : RegisteredRoutes)
	{
		if (ExistingRoute.Get() == Route)
		{
			return;
		}
	}

	RegisteredRoutes.Add(Route);
}

void UPatrolSubsystem::UnregisterPatrolRoute(APatrolRoute* Route)
{
	if (!Route)
	{
		return;
	}

	RegisteredRoutes.RemoveAll([Route](const TWeakObjectPtr<APatrolRoute>& Entry)
	{
		return !Entry.IsValid() || Entry.Get() == Route;
	});
}

APatrolRoute* UPatrolSubsystem::GetPatrolRouteByLocationTag(const FGameplayTag& LocationTag, const FVector& RequesterLocation) const
{
	if (!LocationTag.IsValid())
	{
		return nullptr;
	}

	APatrolRoute* BestRoute = nullptr;
	float MinDistSq = TNumericLimits<float>::Max();

	for (const TWeakObjectPtr<APatrolRoute>& RoutePtr : RegisteredRoutes)
	{
		APatrolRoute* Route = RoutePtr.Get();
		if (!Route || !Route->bIsActive || !Route->HasValidWaypoints())
		{
			continue;
		}

		const bool bTagMatches = Route->LocationTag.MatchesTag(LocationTag)
			|| Route->AdditionalTags.HasTag(LocationTag);

		if (!bTagMatches)
		{
			continue;
		}

		if (RequesterLocation.IsZero())
		{
			return Route;
		}

		const int32 ClosestWaypointIdx = Route->GetClosestWaypointIndex(RequesterLocation);
		if (const AActor* Waypoint = Route->GetWaypoint(ClosestWaypointIdx))
		{
			const float DistSq = FVector::DistSquared(RequesterLocation, Waypoint->GetActorLocation());
			if (DistSq < MinDistSq)
			{
				MinDistSq = DistSq;
				BestRoute = Route;
			}
		}
		else if (!BestRoute)
		{
			BestRoute = Route;
		}
	}

	return BestRoute;
}

TArray<APatrolRoute*> UPatrolSubsystem::GetAllPatrolRoutesByLocationTag(const FGameplayTag& LocationTag) const
{
	TArray<APatrolRoute*> Results;
	if (!LocationTag.IsValid())
	{
		return Results;
	}

	for (const TWeakObjectPtr<APatrolRoute>& RoutePtr : RegisteredRoutes)
	{
		APatrolRoute* Route = RoutePtr.Get();
		if (Route && Route->bIsActive && Route->HasValidWaypoints())
		{
			if (Route->LocationTag.MatchesTag(LocationTag) || Route->AdditionalTags.HasTag(LocationTag))
			{
				Results.Add(Route);
			}
		}
	}

	return Results;
}

TArray<APatrolRoute*> UPatrolSubsystem::GetAllRegisteredRoutes() const
{
	TArray<APatrolRoute*> Results;
	for (const TWeakObjectPtr<APatrolRoute>& RoutePtr : RegisteredRoutes)
	{
		if (APatrolRoute* Route = RoutePtr.Get())
		{
			Results.Add(Route);
		}
	}
	return Results;
}
