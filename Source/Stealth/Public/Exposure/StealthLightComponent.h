#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthLightComponent.generated.h"

class UPointLightComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthLightComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStealthLightComponent();

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable)
	void AddToExposureSystem();
	UFUNCTION(BlueprintCallable)
	void RemoveFromExposureSystem();

protected:
	int32 LightHandle;
	UPROPERTY()
	TObjectPtr<UPointLightComponent> LightComponent;
};
