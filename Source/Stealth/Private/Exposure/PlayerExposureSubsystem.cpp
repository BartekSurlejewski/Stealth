#include "Exposure/PlayerExposureSubsystem.h"

#include "Characters/Player/StealthCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Stealth/Stealth.h"
#include "TimeSystem/TimeSubsystem.h"

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

const float UPlayerExposureSubsystem::CalculatePlayerLightExposure()
{
	//TODO: Add calculating the light exposure when player is in the room during day
	// Return max light exposure during the day
	if (TimeSubsystem != nullptr && TimeSubsystem->IsTimeOfDay(ETimeOfDay::Day))
	{
		return 1.0;
	}

	FVector PlayerPosition = PlayerCharacter->GetActorLocation();

	float Accumulated = 0.f;

	for (const FLightData& LightData : Lights)
	{
		// Phase 1 — broad phase: pure math, no trace
		float Dist = FVector::Dist(PlayerPosition, LightData.Position);
		if (Dist >= LightData.Radius)
		{
			continue;
		}

		float Falloff = 1.f - (Dist / LightData.Radius); // linear; use squared for realism
		Falloff *= LightData.Intensity;

		// Phase 2 — occlusion: line trace only for lights that passed phase 1
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(PlayerCharacter);
		Params.AddIgnoredActor(LightData.OwnerActor);

		bool bBlocked = GetWorld()->LineTraceSingleByChannel(
			Hit, LightData.Position, PlayerPosition, ECC_Visibility, Params);

		//TODO: Remove drawing debug lines
		if (bBlocked)
		{
			DrawDebugLine(GetWorld(), LightData.Position, PlayerPosition, FColor::Red, false, 0.6, 0, 1);
			// not zero — let a tiny bleed through
			Falloff *= 0.005f;
		}
		else
		{
			DrawDebugLine(GetWorld(), LightData.Position, PlayerPosition, FColor::Green, false, 0.3, 0, 1);
		}

		Accumulated = FMath::Clamp(Accumulated + Falloff, 0.f, 1.f);
	}

	return FMath::Clamp(Accumulated, 0.f, 1.f);
}

const float UPlayerExposureSubsystem::CalculateTotalPlayerExposure()
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
