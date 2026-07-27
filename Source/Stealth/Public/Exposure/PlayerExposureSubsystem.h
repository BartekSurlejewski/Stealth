// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/StealthTickableWorldSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "PlayerExposureSubsystem.generated.h"

class UTimeSubsystem;
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
	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;
};

UCLASS()
class STEALTH_API UPlayerExposureSubsystem : public UStealthTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static UPlayerExposureSubsystem* Get(const UObject* WorldContextObject);

	virtual void Tick(float DeltaTime) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

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
	TSoftObjectPtr<UTimeSubsystem> TimeSubsystem;
	UPROPERTY()
	TArray<FLightData> Lights;
	UPROPERTY()
	float CurrentTotalExposure = 1;
	UPROPERTY()
	TObjectPtr<AStealthCharacter> PlayerCharacter;

	UPROPERTY(Transient)
	TArray<float> PartialResultsBuffer; // Reused buffer for multithreaded calculactions
};
