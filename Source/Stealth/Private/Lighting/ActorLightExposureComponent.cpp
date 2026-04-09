#include "Lighting/ActorLightExposureComponent.h"

#include "Lighting/LightExposureSubsystem.h"

UActorLightExposureComponent::UActorLightExposureComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UActorLightExposureComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UActorLightExposureComponent::RecalculateExposure()
{
	ULightExposureSubsystem* LightsRegistry = GetWorld()->GetSubsystem<ULightExposureSubsystem>();

	if (!LightsRegistry)
	{
		return;
	}

	FVector OwnerPosition = GetOwner()->GetActorLocation();

	float Accumulated = 0.f;
	const auto& Lights = LightsRegistry->GetLights();

	for (const FLightData& L : Lights)
	{
		// Phase 1 — broad phase: pure math, no trace
		float Dist = FVector::Dist(OwnerPosition, L.Position);
		if (Dist >= L.Radius)
		{
			continue;
		}

		float Falloff = 1.f - (Dist / L.Radius); // linear; use squared for realism
		Falloff *= L.Intensity;

		// Phase 2 — occlusion: line trace only for lights that passed phase 1
		// if (L.bCastsShadow)
		{
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(GetOwner());

			bool bBlocked = GetWorld()->LineTraceSingleByChannel(
				Hit, L.Position, OwnerPosition, ECC_Visibility, Params);

			if (bBlocked)
			{
				// not zero — let a tiny bleed through
				Falloff *= 0.005f;
			}
		}

		Accumulated = FMath::Clamp(Accumulated + Falloff, 0.f, 1.f);
	}

	TargetExposure = FMath::Clamp(Accumulated, 0.f, 1.f);
}


void UActorLightExposureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TimeSinceLastTick += DeltaTime;
	if (TimeSinceLastTick < TickInterval)
	{
		return;
	}
	TimeSinceLastTick = 0.f;
	RecalculateExposure();

	// Smooth interpolation so the gem doesn't snap
	float Previous = ExposureValue;
	ExposureValue = FMath::FInterpTo(ExposureValue, TargetExposure, TickInterval, 4.f);

	if (!FMath::IsNearlyEqual(ExposureValue, Previous, 0.01f))
	{
		OnExposureChanged.Broadcast(ExposureValue);
	}
}
