#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActorLightExposureComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExposureChanged, float, NewValue);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UActorLightExposureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UActorLightExposureComponent();

	UPROPERTY(BlueprintAssignable)
	FOnExposureChanged OnExposureChanged;

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	void RecalculateExposure();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UPROPERTY(EditAnywhere)
	float TickInterval = 0.1f;
	UPROPERTY()
	float TimeSinceLastTick = 0.0f;
	UPROPERTY(BlueprintReadOnly)
	float ExposureValue = 0.0f;

protected:
	UPROPERTY()
	float TargetExposure = 0.0f;

public:
	[[nodiscard]] float GetExposure() const { return ExposureValue; }
};
