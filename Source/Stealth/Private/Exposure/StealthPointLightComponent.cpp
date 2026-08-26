#include "Exposure/StealthPointLightComponent.h"

#include "Components/PointLightComponent.h"
#include "Exposure/PlayerExposureSubsystem.h"
#include "Stealth/Stealth.h"


void UStealthPointLightComponent::AddToExposureSystem()
{
	Super::AddToExposureSystem();

	UPlayerExposureSubsystem* ExposureSubsystem = UPlayerExposureSubsystem::Get(this);

	if (!ExposureSubsystem)
	{
		return;
	}


	UPointLightComponent* LightComponent = GetOwner()->FindComponentByClass<UPointLightComponent>();
	if (!LightComponent)
	{
		UE_LOG(LogStealth, Warning, TEXT("StealthLightComponent: No UPointLightComponent on %s"), *GetOwner()->GetName());
		return;
	}


	LightHandle = ExposureSubsystem->RegisterPointLight(LightComponent);
}

void UStealthPointLightComponent::RemoveFromExposureSystem()
{
	if (UPlayerExposureSubsystem* ExposureSubsystem = UPlayerExposureSubsystem::Get(this))
	{
		ExposureSubsystem->UnregisterPointLight(LightHandle);
	}

	Super::RemoveFromExposureSystem();
}
