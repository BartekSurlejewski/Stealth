#include "Exposure/StealthLightComponent.h"

#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Exposure/PlayerExposureSubsystem.h"
#include "Stealth/Stealth.h"


UStealthLightComponent::UStealthLightComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStealthLightComponent::BeginPlay()
{
	Super::BeginPlay();

	AddToExposureSystem();
}

void UStealthLightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveFromExposureSystem();

	Super::EndPlay(EndPlayReason);
}

void UStealthLightComponent::AddToExposureSystem() {}

void UStealthLightComponent::RemoveFromExposureSystem() {}
