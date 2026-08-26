#pragma once

#include "CoreMinimal.h"
#include "StealthLightComponent.h"
#include "StealthSpotLightComponent.generated.h"

USTRUCT()
struct FSpotLightData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;
	float Radius;
	float Intensity;
	float ConeHalfAngleDegrees = 45.f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthSpotLightComponent : public UStealthLightComponent
{
	GENERATED_BODY()

protected:
	virtual void AddToExposureSystem() override;
	virtual void RemoveFromExposureSystem() override;
};
