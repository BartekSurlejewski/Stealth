#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Templates/SubclassOf.h"
#include "PlayerMovementAbilitySet.generated.h"

class UGameplayAbility;
class UInputAction;

UENUM(BlueprintType)
enum class EMovementAbilityInputMode : uint8
{
	Hold,        // single InputAction, Started activates, Completed ends
	Toggle,      // single InputAction, each press flips active/inactive
	TwoActions   // separate enable/disable InputActions
};

USTRUCT(BlueprintType)
struct STEALTH_API FMovementAbilityBinding
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Ability")
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Ability")
	TSubclassOf<UGameplayAbility> AbilityClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Ability")
	TObjectPtr<UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Ability")
	EMovementAbilityInputMode ActivationType = EMovementAbilityInputMode::Hold;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Ability", meta = (EditCondition = "ActivationType == EMovementAbilityInputMode::TwoActions", EditConditionHides))
	TObjectPtr<UInputAction> InputActionDisable = nullptr;
};

UCLASS(BlueprintType)
class STEALTH_API UPlayerMovementAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Abilities")
	TArray<FMovementAbilityBinding> AbilityBindings;
};
