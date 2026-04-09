// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LightExposureSubsystem.generated.h"

class UActorLightExposureComponent;
class UWorld;

USTRUCT()
struct FLightData
{
	GENERATED_BODY()

	FVector Position;
	float Radius;
	float Intensity;
};

UCLASS()
class STEALTH_API ULightExposureSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION()
	int32 RegisterLight(const FLightData& LightData);
	UFUNCTION()
	void UnregisterLight(int32 Handle);
	UFUNCTION()
	void UpdateLight(int32 Handle, const FLightData& LightData);

	UFUNCTION()
	const TArray<FLightData>& GetLights() const { return Lights; }

	UFUNCTION(BlueprintCallable)
	const float GetPlayerLightExposure();

private:
	UPROPERTY()
	TArray<FLightData> Lights;
	UPROPERTY()
	TArray<int32> FreeHandles;
	UPROPERTY()
	TObjectPtr<UActorLightExposureComponent> PlayerlightExposureComponent;
};
