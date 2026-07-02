#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "StealthWorldSubsystem.generated.h"


class AStealthWorldSettings;

UCLASS(Abstract)
class STEALTH_API UStealthWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	UPROPERTY()
	TObjectPtr<AStealthWorldSettings> WorldSettings;
};
