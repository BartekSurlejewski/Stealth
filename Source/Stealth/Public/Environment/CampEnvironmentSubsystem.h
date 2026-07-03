#pragma once

#include "CoreMinimal.h"
#include "Core/StealthWorldSubsystem.h"
#include "CampEnvironmentSubsystem.generated.h"

class ATargetPoint;

UCLASS()
class STEALTH_API UCampEnvironmentSubsystem : public UStealthWorldSubsystem
{
	GENERATED_BODY()

	/*Methods*/
public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UFUNCTION(BlueprintCallable)
	ATargetPoint* GetRandomPointOfInterest();
};
