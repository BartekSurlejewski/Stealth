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


	UPlayerExposureSubsystem* ExposureSubsystem = UPlayerExposureSubsystem::Get(this);


	if (!ExposureSubsystem)
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

	LightHandle = ExposureSubsystem->RegisterLight(LightData);
}

void UStealthLightComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UPlayerExposureSubsystem* ExposureSubsystem = UPlayerExposureSubsystem::Get(this))
	{
		ExposureSubsystem->UnregisterLight(LightHandle);
	}

	Super::EndPlay(EndPlayReason);
}
