#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "PlayerMovementAbilityComponent.generated.h"

class ACharacter;
class UAbilitySystemComponent;
class UCharacterMovementComponent;
class UEnhancedInputComponent;
class UPlayerMovementAbilitySet;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UPlayerMovementAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerMovementAbilityComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void BindInput(UEnhancedInputComponent* EIC);

	void HandleAbilityPressed(FGameplayTag AbilityTag);
	void HandleAbilityReleased(FGameplayTag AbilityTag);
	void HandleAbilityToggled(FGameplayTag AbilityTag);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Abilities")
	TObjectPtr<UPlayerMovementAbilitySet> MovementAbilitySet;

	UPROPERTY()
	TObjectPtr<ACharacter> OwningCharacter;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CharacterMovementComponent;

	UPROPERTY()
	TMap<FGameplayTag, FGameplayAbilitySpecHandle> AbilitySpecHandles;

private:
	void OnSpeedAttributeChanged(const FOnAttributeChangeData& Data);
	void OnJumpVelocityAttributeChanged(const FOnAttributeChangeData& Data);

	FDelegateHandle SpeedAttributeChangedDelegateHandle;
	FDelegateHandle JumpVelocityAttributeChangedDelegateHandle;

	bool bWasFalling = false;
	bool bWasMoving = false;
};
