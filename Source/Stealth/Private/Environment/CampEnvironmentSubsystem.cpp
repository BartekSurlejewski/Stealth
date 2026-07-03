#include "Environment/CampEnvironmentSubsystem.h"

#include "Core/Data/StealthWorldSettings.h"
#include "Engine/TargetPoint.h"

void UCampEnvironmentSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	BuildWorkingCopiesFromSettings();
}

ATargetPoint* UCampEnvironmentSubsystem::GetRandomPointOfInterest()
{
	if (PointsOfInterest.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No POIs in camp environment"));
		return nullptr;
	}

	auto newPoi = PointsOfInterest[FMath::RandRange(0, PointsOfInterest.Num() - 1)];

	UE_LOG(LogTemp, Warning, TEXT("Random POI: %s"), *newPoi->GetName());

	return newPoi;
}

void UCampEnvironmentSubsystem::BuildWorkingCopiesFromSettings()
{
	if (!WorldSettings)
	{
		return;
	}

	PointsOfInterest.Reset();
	PointsOfInterest.Reserve(WorldSettings->GetPointsOfInterest().Num());

	for (const auto& PointOfInterest : WorldSettings->GetPointsOfInterest())
	{
		if (!IsValid(PointOfInterest))
		{
			continue;
		}

		ATargetPoint* WorkingCopy = DuplicateObject<ATargetPoint>(PointOfInterest, this);
		PointsOfInterest.Add(WorkingCopy);
	}
}
