#include "Exposure/StealthSpotLightComponent.h"

#include "Components/SpotLightComponent.h"
#include "Exposure/PlayerExposureSubsystem.h"
#include "Stealth/Stealth.h"

void UStealthSpotLightComponent::AddToExposureSystem()
{
	Super::AddToExposureSystem();

	UPlayerExposureSubsystem* ExposureSubsystem = UPlayerExposureSubsystem::Get(this);

	if (!ExposureSubsystem)
	{
		return;
	}

	USpotLightComponent* LightComponent = GetOwner()->FindComponentByClass<USpotLightComponent>();
	if (!LightComponent)
	{
		UE_LOG(LogStealth, Warning, TEXT("StealthLightComponent: No UPointLightComponent on %s"), *GetOwner()->GetName());
		return;
	}

	LightHandle = ExposureSubsystem->RegisterSpotLight(LightComponent);
}

void UStealthSpotLightComponent::RemoveFromExposureSystem()
{
	if (UPlayerExposureSubsystem* ExposureSubsystem = UPlayerExposureSubsystem::Get(this))
	{
		ExposureSubsystem->UnregisterSpotLight(LightHandle);
	}

	Super::RemoveFromExposureSystem();
}
