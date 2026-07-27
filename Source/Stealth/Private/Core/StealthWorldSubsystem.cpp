#include "Core/StealthWorldSubsystem.h"

#include "Core/Data/StealthWorldSettings.h"
#include "Engine/World.h"
#include "Stealth/Stealth.h"


bool UStealthWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	if (!World)
	{
		UE_LOG(LogStealth, Warning, TEXT("[Stealth World Subsystem] Not Creating - world is null"))

		return false;
	}

	const AStealthWorldSettings* rrhWorldSettings = Cast<AStealthWorldSettings>(World->GetWorldSettings());
	if (!rrhWorldSettings || !rrhWorldSettings->IsGameplayWorld())
	{
		return false;
	}

	return true;
}

void UStealthWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	WorldSettings = Cast<AStealthWorldSettings>(GetWorld()->GetWorldSettings());
	if (!WorldSettings)
	{
		UE_LOG(LogStealth, Error, TEXT("[Stealth World Subsystem] No world settings"))
	}
}
