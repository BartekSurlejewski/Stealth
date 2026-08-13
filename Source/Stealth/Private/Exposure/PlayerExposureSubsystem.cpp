#include "Exposure/PlayerExposureSubsystem.h"

#include "DrawDebugHelpers.h"
#include "Async/ParallelFor.h"
#include "Characters/Player/StealthCharacter.h"
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

int32 UPlayerExposureSubsystem::RegisterLight(const FLightData& LightData)
{
	int32 Handle = -1;

	Handle = Lights.Add(LightData);

	return Handle;
}

void UPlayerExposureSubsystem::UnregisterLight(int32 Handle)
{
	if (!Lights.IsValidIndex(Handle))
	{
		return;
	}

	Lights[Handle] = FLightData();
}

void UPlayerExposureSubsystem::UpdateLight(int32 Handle, const FLightData& LightData)
{
	if (!Lights.IsValidIndex(Handle))
	{
		return;
	}

	Lights[Handle] = LightData;
}

float UPlayerExposureSubsystem::CalculatePlayerLightExposure()
{
	//TODO: Add calculating the light exposure when player is in the room during day
	// Return max light exposure during the day
	if (TimeSubsystem != nullptr && TimeSubsystem->IsTimeOfDay(ETimeOfDay::Day))
	{
		return 1.0;
	}

	FVector PlayerPosition = PlayerCharacter->GetActorLocation();

	PartialResultsBuffer.SetNumZeroed(Lights.Num(), EAllowShrinking::No);

	ParallelFor(Lights.Num(), [&](int32 index)
	{
		auto LightData = Lights[index];
		// Phase 1 — broad phase: pure math, no trace
		float Dist = FVector::Dist(PlayerPosition, LightData.Position);
		if (Dist >= LightData.Radius)
		{
			return;
		}

		float LightIntensity = 1.f - (Dist / LightData.Radius); // linear; use squared for realism
		LightIntensity *= LightData.Intensity;

		// Phase 2 — occlusion: line trace only for lights that passed phase 1
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(PlayerCharacter);
		Params.AddIgnoredActor(LightData.OwnerActor);

		bool bBlocked = GetWorld()->LineTraceSingleByChannel(
			Hit, LightData.Position, PlayerPosition, ECC_Visibility, Params);

		if (bBlocked)
		{
			// not zero — let a tiny bleed through
			LightIntensity *= 0.005f;
		}

		PartialResultsBuffer[index] = LightIntensity;
	});

	float Accumulated = 0.f;
	for (const float PartialResult : PartialResultsBuffer)
	{
		Accumulated += PartialResult;
	}

	return FMath::Clamp(Accumulated, 0.f, 1.f);
}

float UPlayerExposureSubsystem::CalculateTotalPlayerExposure()
{
	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<AStealthCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

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
