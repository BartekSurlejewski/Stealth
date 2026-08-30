#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "StealthPlayerGameplayAbility.generated.h"

class AStealthPlayerController;
class AStealthPlayerCharacter;

USTRUCT(BlueprintType)
struct FGameplayAbilityStealthPlayerInfo : public FGameplayAbilityActorInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<AStealthPlayerCharacter> StealthPlayerCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "ActorInfo")
	TWeakObjectPtr<AStealthPlayerController> StealthPlayerController;
};

UCLASS(Abstract)
class STEALTH_API UStealthPlayerGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	// Helper function to fetch the struct cleanly in C++ code
	const FGameplayAbilityStealthPlayerInfo* GetStealthPlayerInfo() const;

	// Exposes custom pointers directly to Blueprint graphs
	UFUNCTION(BlueprintPure, Category = "Ability|ActorInfo")
	AStealthPlayerCharacter* GetPlayerCharacter() const;
};
