// Fill out your copyright notice in the Description page of Project Settings.


#include "Lighting/LightExposureSubsystem.h"

#include "Characters/Player/StealthCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Lighting/ActorLightExposureComponent.h"

int32 ULightExposureSubsystem::RegisterLight(const FLightData& LightData)
{
	int32 Handle = -1;

	Handle = Lights.Add(LightData);

	return Handle;
}

void ULightExposureSubsystem::UnregisterLight(int32 Handle)
{
	if (!Lights.IsValidIndex(Handle))
	{
		return;
	}

	Lights[Handle] = FLightData();
}

void ULightExposureSubsystem::UpdateLight(int32 Handle, const FLightData& LightData)
{
	if (!Lights.IsValidIndex(Handle))
	{
		return;
	}

	Lights[Handle] = LightData;
}

const float ULightExposureSubsystem::GetPlayerLightExposure()
{
	if (!PlayerlightExposureComponent)
	{
		AStealthCharacter* PlayerCharacter = Cast<AStealthCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
		if (!PlayerCharacter)
		{
			return 1;
		}

		PlayerlightExposureComponent = PlayerCharacter->GetLightExposureComponent();
	}

	return PlayerlightExposureComponent->GetExposure();
}
