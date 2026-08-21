#pragma once

#include "CoreMinimal.h"
#include "Core/StealthTickableWorldSubsystem.h"
#include "LegalitySubsystem.generated.h"

/**
 * System for determining the legality of actions in the game.
 */
UCLASS()
class STEALTH_API ULegalitySubsystem : public UStealthTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	static ULegalitySubsystem* Get(const UObject* WorldContextObject)
	{
		return UStealthTickableWorldSubsystem::Get<ULegalitySubsystem>(WorldContextObject);
	}

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	bool IsPlayerPerformingIllegalAction() const { return bIsPlayerPerformingIllegalAction; }

private:
	UPROPERTY()
	bool bIsPlayerPerformingIllegalAction = false;

	// Tick interval
	UPROPERTY()
	float TimeSinceLastTick = 0.0f;
	static constexpr float TickInterval = 0.2f;
};
