#include "Exposure/PlayerExposureSubsystem.h"

#include "DrawDebugHelpers.h"
#include "Async/ParallelFor.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Stealth/Stealth.h"
#include "TimeSystem/TimeSubsystem.h"

UPlayerExposureSubsystem* UPlayerExposureSubsystem::Get(const UObject* WorldContextObject)
{
	return UStealthTickableWorldSubsystem::Get<UPlayerExposureSubsystem>(WorldContextObject);
}

void UPlayerExposureSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentTotalExposure = CalculateTotalPlayerExposure();
}

void UPlayerExposureSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	TimeSubsystem = GetWorld()->GetSubsystem<UTimeSubsystem>();
}

int32 UPlayerExposureSubsystem::RegisterPointLight(UPointLightComponent* LightComponent)
{
	int32 Handle = -1;

	Handle = PointLights.Add(LightComponent);

	return Handle;
}

void UPlayerExposureSubsystem::UnregisterPointLight(const int32 Handle)
{
	if (!PointLights.IsValidIndex(Handle))
	{
		return;
	}

	PointLights[Handle] = nullptr;
}

int32 UPlayerExposureSubsystem::RegisterSpotLight(USpotLightComponent* LightComponent)
{
	int32 Handle = -1;

	Handle = SpotLights.Add(LightComponent);

	return Handle;
}

void UPlayerExposureSubsystem::UnregisterSpotLight(int32 Handle)
{
	if (!SpotLights.IsValidIndex(Handle))
	{
		return;
	}

	SpotLights[Handle] = nullptr;
}

float UPlayerExposureSubsystem::CalculatePlayerLightExposure()
{
	if (TimeSubsystem != nullptr && TimeSubsystem->IsTimeOfDay(ETimeOfDay::Day))
	{
		return 1.0;
	}

	FVector PlayerPosition = PlayerCharacter->GetActorLocation();

	float Accumulated = 0.f;
	Accumulated += CalculatePointLightsExposure(PlayerPosition);
	Accumulated += CalculateSpotLightsExposure(PlayerPosition);

	return FMath::Clamp(Accumulated, 0.f, 1.f);
}

float UPlayerExposureSubsystem::CalculatePointLightsExposure(const FVector& PlayerLocation)
{
	PointLightPartialResults.SetNumZeroed(PointLights.Num(), EAllowShrinking::No);

	ParallelFor(PointLights.Num(), [&](int32 index)
	{
		PointLightPartialResults[index] = 0.0f;

		if (const UPointLightComponent* LightComponent = PointLights[index].Get())
		{
			// Phase 1 — distance
			float Dist = FVector::Dist(PlayerLocation, LightComponent->GetComponentLocation());
			if (Dist >= LightComponent->AttenuationRadius)
			{
				return;
			}

			float LightIntensity = 1.f - (Dist / LightComponent->AttenuationRadius);
			LightIntensity *= LightComponent->Intensity;

			// Phase 2 — occlusion
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(PlayerCharacter);
			AActor* LightOwner = LightComponent->GetOwner();
			Params.AddIgnoredActor(LightOwner);
			if (AActor* ParentActor = LightOwner->GetParentActor())
			{
				Params.AddIgnoredActor(ParentActor);
			}

			FCollisionResponseParams ResponseParams(ECR_Block); // default: block everything
			ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Ignore);

			bool bBlocked = GetWorld()->LineTraceSingleByChannel(
				Hit, LightComponent->GetComponentLocation(), PlayerLocation, ECC_Visibility, Params, ResponseParams);

			if (bBlocked)
			{
				LightIntensity *= 0.0;
			}

			PointLightPartialResults[index] = LightIntensity;
		}
	});

	float Accumulated = 0.f;
	for (const float PartialResult : PointLightPartialResults)
	{
		Accumulated += PartialResult;
	}
	return Accumulated;
}

float UPlayerExposureSubsystem::CalculateSpotLightsExposure(const FVector& PlayerLocation)
{
	SpotLightPartialResults.SetNumZeroed(SpotLights.Num(), EAllowShrinking::No);

	ParallelFor(SpotLights.Num(), [&](int32 index)
	{
		SpotLightPartialResults[index] = 0.0f;

		if (const USpotLightComponent* LightComponent = SpotLights[index].Get())
		{
			const FVector LightLocation = LightComponent->GetComponentLocation();

			// Phase 1 — distance (broad phase, pure math)
			float Dist = FVector::Dist(PlayerLocation, LightLocation);
			if (Dist >= LightComponent->AttenuationRadius)
			{
				return;
			}

			// Phase 2 — angle (still pure math, no trace)
			// OuterConeAngle/InnerConeAngle on USpotLightComponent are already half-angles in degrees.
			FVector ToPlayer = (PlayerLocation - LightLocation).GetSafeNormal();
			FVector LightForward = LightComponent->GetForwardVector();

			float CosAngleToPlayer = FVector::DotProduct(LightForward, ToPlayer);
			float CosOuterCone = FMath::Cos(FMath::DegreesToRadians(LightComponent->OuterConeAngle));

			if (CosAngleToPlayer < CosOuterCone)
			{
				// Outside the cone entirely
				return;
			}

			float LightIntensity = 1.f - (Dist / LightComponent->AttenuationRadius);

			// Soft edge: fade between outer and inner cone, matching the light's own falloff
			float CosInnerCone = FMath::Cos(FMath::DegreesToRadians(LightComponent->InnerConeAngle));
			float AngleFalloff = (CosAngleToPlayer - CosOuterCone) / FMath::Max(CosInnerCone - CosOuterCone, KINDA_SMALL_NUMBER);
			LightIntensity *= FMath::Clamp(AngleFalloff, 0.f, 1.f);

			LightIntensity *= LightComponent->Intensity;

			// Phase 3 — occlusion: raycast only for lights that passed distance + angle
			FHitResult Hit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(PlayerCharacter);
			AActor* LightOwner = LightComponent->GetOwner();
			Params.AddIgnoredActor(LightOwner);
			if (AActor* ParentActor = LightOwner->GetParentActor())
			{
				Params.AddIgnoredActor(ParentActor);
			}

			FCollisionResponseParams ResponseParams(ECR_Block); // default: block everything
			ResponseParams.CollisionResponse.SetResponse(ECC_Pawn, ECR_Ignore);

			bool bBlocked = GetWorld()->LineTraceSingleByChannel(
				Hit, LightLocation, PlayerLocation, ECC_Visibility, Params, ResponseParams);

			if (bBlocked)
			{
				LightIntensity *= 0.0;
			}

			SpotLightPartialResults[index] = LightIntensity;
		}
	});

	float Accumulated = 0.f;
	for (const float PartialResult : SpotLightPartialResults)
	{
		Accumulated += PartialResult;
	}
	return Accumulated;
}

float UPlayerExposureSubsystem::CalculateTotalPlayerExposure()
{
	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<AStealthPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

		if (!PlayerCharacter)
		{
			UE_LOG(LogStealth, Error, TEXT("PlayerExposure: No player character"));

			return 1;
		}
	}

	float LightExposure = CalculatePlayerLightExposure();

	float TotalExposure = LightExposure * (PlayerCharacter->IsCrouched() ? 0.5f : 1); //TODO: move to some general gameplay settings;

	return TotalExposure;
}
