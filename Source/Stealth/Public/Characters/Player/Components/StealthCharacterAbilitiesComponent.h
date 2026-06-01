#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "StealthCharacterAbilitiesComponent.generated.h"

class UGameplayEffect;
class UGameplayAbility;
class UStealthCharacterAttributeSet;
class AStealthCharacter;
class UAbilitySystemComponent;

// Handles the GAS abilities connected to StealthCharacter and the character status
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthCharacterAbilitiesComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TObjectPtr<AStealthCharacter> OwningCharacter;
	UPROPERTY(VisibleAnywhere, Category="Ability System", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, Category = "Ability System", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStealthCharacterAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
	FGameplayTag SprintAbilityTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
	FGameplayTag CrouchAbilityTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
	FGameplayTag JumpAbilityTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
	FGameplayTag IsMovingTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
	FGameplayTag StaminaRegenTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
	FGameplayTag IsFallingTag;

	UPROPERTY()
	FGameplayTagContainer SprintAbilityTagsContainer;
	UPROPERTY()
	FGameplayTagContainer CrouchAbilityTagsContainer;
	UPROPERTY()
	FGameplayTagContainer JumpAbilityTagsContainer;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability System", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability System", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UGameplayEffect>> StartingEffects;

public:
	UStealthCharacterAbilitiesComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Ability System")
	TArray<FGameplayAbilitySpecHandle> GrantAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant);
	UFUNCTION(BlueprintCallable, Category="Ability System")
	void RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove);
	UFUNCTION(BlueprintCallable, Category="Ability System")
	TArray<FActiveGameplayEffectHandle> ApplyGameplayEffectsToSelf(TArray<TSubclassOf<UGameplayEffect>> EffectsToApply);
	UFUNCTION(BlueprintCallable, Category="Ability System")
	void RemoveGameplayEffects(TArray<FActiveGameplayEffectHandle> EffectHandlesToRemove);

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
