#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "StealthCharacterBehaviourComponent.generated.h"

class UStealthCharacterAttributeSet;
class AStealthCharacter;
class UAbilitySystemComponent;

// Handles the GAS abilities connected to StealthCharacter and the character status
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthCharacterBehaviourComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TWeakObjectPtr<AStealthCharacter> OwningCharacter;
	UPROPERTY(VisibleAnywhere, Category="Ability System", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, Category = "Ability System")
	TWeakObjectPtr<UStealthCharacterAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag SprintAbilityTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag CrouchAbilityTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag JumpAbilityTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag IsMovingTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag StaminaRegenTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag IsFallingTag;

	UPROPERTY()
	FGameplayTagContainer SprintAbilityTagsContainer;
	UPROPERTY()
	FGameplayTagContainer CrouchAbilityTagsContainer;
	UPROPERTY()
	FGameplayTagContainer JumpAbilityTagsContainer;

public:
	UStealthCharacterBehaviourComponent();

protected:

public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintInputStart();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintInputEnd();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchInputStart();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchInputEnd();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

private:
	UFUNCTION()
	void EndSprint() const;
	UFUNCTION()
	void UpdateTags() const;
};
