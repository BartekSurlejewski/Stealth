#pragma once

#include "CoreMinimal.h"
#include "StealthLightComponent.h"
#include "StealthPointLightComponent.generated.h"


USTRUCT()
struct FPointLightData
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;
	EStealthLightType LightType;
	float Radius;
	float Intensity;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthPointLightComponent : public UStealthLightComponent
{
	GENERATED_BODY()

protected:
	virtual void AddToExposureSystem() override;
	virtual void RemoveFromExposureSystem() override;
};
