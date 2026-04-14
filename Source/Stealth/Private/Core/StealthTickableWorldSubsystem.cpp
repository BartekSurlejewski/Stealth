// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/StealthTickableWorldSubsystem.h"

#include "Core/Data/StealthWorldSettings.h"
#include "Stealth/Stealth.h"

bool UStealthTickableWorldSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	if (!World)
	{
		UE_LOG(LogStealth, Warning, TEXT("[Stealth Tickable World Subsystem] Not Creating - world is null"))

		return false;
	}

	const AStealthWorldSettings* rrhWorldSettings = Cast<AStealthWorldSettings>(World->GetWorldSettings());
	if (!rrhWorldSettings || !rrhWorldSettings->IsGameplayWorld())
	{
		return false;
	}

	return true;
}

void UStealthTickableWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	WorldSettings = Cast<AStealthWorldSettings>(GetWorld()->GetWorldSettings());
	if (!WorldSettings)
	{
		UE_LOG(LogStealth, Error, TEXT("[StealthTickableWorldSubsystem] No world settings"))
	}
}
