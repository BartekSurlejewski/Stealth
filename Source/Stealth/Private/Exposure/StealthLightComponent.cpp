#include "Exposure/StealthLightComponent.h"

#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Exposure/PlayerExposureSubsystem.h"
#include "Stealth/Stealth.h"


UStealthLightComponent::UStealthLightComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UStealthLightComponent::BeginPlay()
{
	Super::BeginPlay();

	UPlayerExposureSubsystem* LightsRegistry = GetWorld()->GetSubsystem<UPlayerExposureSubsystem>();

	if (!LightsRegistry)
	{
		return;
	}

	if (!LightComponent)
	{
		LightComponent = GetOwner()->FindComponentByClass<UPointLightComponent>();
		if (!LightComponent)
		{
			UE_LOG(LogStealth, Warning, TEXT("StealthLightComponent: No UPointLightComponent on %s"), *GetOwner()->GetName());
			return;
		}
	}

	FLightData LightData;
	if (LightComponent)
	{
		LightData.Position = LightComponent->GetComponentLocation();
	}
	else
	{
		LightData.Position = GetOwner()->GetActorLocation();
	}
	LightData.Radius = LightComponent->AttenuationRadius;
	LightData.Intensity = LightComponent->Intensity;
	LightData.OwnerActor = GetOwner();

	LightHandle = LightsRegistry->RegisterLight(LightData);
}

void UStealthLightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UPlayerExposureSubsystem* LightsRegistry = GetWorld()->GetSubsystem<UPlayerExposureSubsystem>();
	if (LightsRegistry)
	{
		LightsRegistry->UnregisterLight(LightHandle);
	}

	Super::EndPlay(EndPlayReason);
}
