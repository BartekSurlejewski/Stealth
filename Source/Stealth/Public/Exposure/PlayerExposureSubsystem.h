// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/StealthTickableWorldSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "PlayerExposureSubsystem.generated.h"

class AStealthCharacter;
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
class STEALTH_API UPlayerExposureSubsystem : public UStealthTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	int32 RegisterLight(const FLightData& LightData);
	UFUNCTION()
	void UnregisterLight(int32 Handle);
	UFUNCTION()
	void UpdateLight(int32 Handle, const FLightData& LightData);

	UFUNCTION(BlueprintCallable)
	[[nodiscard]] float GetCurrentTotalExposure() const { return CurrentTotalExposure; }

private:
	UFUNCTION(BlueprintCallable)
	const float CalculatePlayerLightExposure();
	UFUNCTION(BlueprintCallable)
	const float CalculateTotalPlayerExposure();

private:
	UPROPERTY()
	TArray<FLightData> Lights;
	UPROPERTY()
	TArray<int32> FreeHandles;
	UPROPERTY()
	float CurrentTotalExposure = 1;
	UPROPERTY()
	TObjectPtr<AStealthCharacter> PlayerCharacter;

public:
	// UPROPERTY()
	// TObjectPtr<UActorLightExposureComponent> PlayerlightExposureComponent;
};
