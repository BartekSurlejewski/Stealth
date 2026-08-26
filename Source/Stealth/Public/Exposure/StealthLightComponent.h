#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthLightComponent.generated.h"

class UPointLightComponent;

UENUM(BlueprintType)
enum class EStealthLightType : uint8
{
	PointLight,
	SpotLight
};

UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthLightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStealthLightComponent();

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	virtual void AddToExposureSystem();
	UFUNCTION(BlueprintCallable)
	virtual void RemoveFromExposureSystem();

protected:
	UPROPERTY()
	int32 LightHandle;
};
