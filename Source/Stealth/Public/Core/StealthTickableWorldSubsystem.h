#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
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

	template <typename T>
	static T* Get(const UObject* WorldContextObject)
	{
		static_assert(TIsDerivedFrom<T, UStealthTickableWorldSubsystem>::IsDerived, "T must derive from UStealthTickableWorldSubsystem");

		if (!WorldContextObject)
		{
			return nullptr;
		}

		const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
		if (!World)
		{
			return nullptr;
		}

		return World->GetSubsystem<T>();
	}

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
	UPROPERTY()
	TObjectPtr<AStealthWorldSettings> WorldSettings;
};
