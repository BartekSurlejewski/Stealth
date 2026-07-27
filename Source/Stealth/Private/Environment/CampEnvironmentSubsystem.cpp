#include "Environment/CampEnvironmentSubsystem.h"

#include "Core/Data/StealthWorldSettings.h"
#include "Engine/TargetPoint.h"


void UCampEnvironmentSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
}

ATargetPoint* UCampEnvironmentSubsystem::GetRandomPointOfInterest()
{
	if (!WorldSettings || WorldSettings->GetPointsOfInterest().IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("No POIs in camp environment"));

		return nullptr;
	}

	return WorldSettings->GetPointsOfInterest()[FMath::RandRange(0, WorldSettings->GetPointsOfInterest().Num() - 1)];
}
