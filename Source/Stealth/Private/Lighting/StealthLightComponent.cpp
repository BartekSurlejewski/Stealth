#include "Lighting/StealthLightComponent.h"

#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Lighting/LightExposureSubsystem.h"
#include "Stealth/Stealth.h"


UStealthLightComponent::UStealthLightComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UStealthLightComponent::BeginPlay()
{
	Super::BeginPlay();

	ULightExposureSubsystem* LightsRegistry = GetWorld()->GetSubsystem<ULightExposureSubsystem>();

	if (!LightsRegistry)
	{
		return;
	}

	UPointLightComponent* PointLightComponent = GetOwner()->FindComponentByClass<UPointLightComponent>();
	if (!PointLightComponent)
	{
		UE_LOG(LogStealth, Warning, TEXT("StealthLightComponent: No UPointLightComponent on %s"), *GetOwner()->GetName());
		return;
	}

	FLightData LightData;
	LightData.Position = GetOwner()->GetActorLocation();
	LightData.Radius = PointLightComponent->AttenuationRadius;
	LightData.Intensity = PointLightComponent->Intensity;

	LightHandle = LightsRegistry->RegisterLight(LightData);
}

void UStealthLightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ULightExposureSubsystem* LightsRegistry = GetWorld()->GetSubsystem<ULightExposureSubsystem>();
	if (LightsRegistry)
	{
		LightsRegistry->UnregisterLight(LightHandle);
	}

	Super::EndPlay(EndPlayReason);
}
