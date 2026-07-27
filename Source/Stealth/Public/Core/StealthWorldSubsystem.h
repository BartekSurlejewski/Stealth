#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/WorldSubsystem.h"
#include "StealthWorldSubsystem.generated.h"


class AStealthWorldSettings;

UCLASS(Abstract)
class STEALTH_API UStealthWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	template <typename T>
	static T* Get(const UObject* WorldContextObject)
	{
		static_assert(TIsDerivedFrom<T, UStealthWorldSubsystem>::IsDerived, "T must derive from UStealthWorldSubsystem");

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
