#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "StealthTickableWorldSubsystem.generated.h"

class AStealthWorldSettings;

UCLASS(Abstract)
class STEALTH_API UStealthTickableWorldSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual TStatId GetStatId() const override
	{
		return GetStatID();
	}

	virtual bool IsTickable() const override { return true; }

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	UPROPERTY()
	TObjectPtr<AStealthWorldSettings> WorldSettings;
};
