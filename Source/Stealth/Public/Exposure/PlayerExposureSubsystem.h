#pragma once

#include "CoreMinimal.h"
#include "Core/StealthTickableWorldSubsystem.h"
#include "PlayerExposureSubsystem.generated.h"

class USpotLightComponent;
class UPointLightComponent;
struct FPointLightData;
class UTimeSubsystem;
class AStealthPlayerCharacter;
class UActorLightExposureComponent;
class UWorld;

UCLASS()
class STEALTH_API UPlayerExposureSubsystem : public UStealthTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static UPlayerExposureSubsystem* Get(const UObject* WorldContextObject);

	virtual void Tick(float DeltaTime) override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UFUNCTION()
	int32 RegisterPointLight(UPointLightComponent* LightComponent);
	UFUNCTION()
	void UnregisterPointLight(int32 Handle);

	UFUNCTION()
	int32 RegisterSpotLight(USpotLightComponent* LightComponent);
	UFUNCTION()
	void UnregisterSpotLight(int32 Handle);

	UFUNCTION(BlueprintCallable)
	[[nodiscard]] float GetCurrentTotalExposure() const { return CurrentTotalExposure; }

private:
	UFUNCTION(BlueprintCallable)
	float CalculatePlayerLightExposure();
	UFUNCTION()
	float CalculatePointLightsExposure(const FVector& PlayerLocation);
	UFUNCTION()
	float CalculateSpotLightsExposure(const FVector& PlayerLocation);
	UFUNCTION(BlueprintCallable)
	float CalculateTotalPlayerExposure();

private:
	UPROPERTY()
	TSoftObjectPtr<UTimeSubsystem> TimeSubsystem;
	UPROPERTY()
	TArray<TWeakObjectPtr<UPointLightComponent>> PointLights;
	UPROPERTY()
	TArray<TWeakObjectPtr<USpotLightComponent>> SpotLights;
	UPROPERTY()
	float CurrentTotalExposure = 1;
	UPROPERTY()
	TObjectPtr<AStealthPlayerCharacter> PlayerCharacter;

	UPROPERTY(Transient)
	TArray<float> PointLightPartialResults;
	UPROPERTY(Transient)
	TArray<float> SpotLightPartialResults;
};
