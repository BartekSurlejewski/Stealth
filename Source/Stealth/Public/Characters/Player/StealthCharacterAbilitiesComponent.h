// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "StealthCharacterAbilitiesComponent.generated.h"


class UStealthCharacterAttributeSet;
class AStealthCharacter;
class UAbilitySystemComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthCharacterAbilitiesComponent : public UActorComponent
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
	FGameplayTag MovingTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag StaminaRegenTag;

	UPROPERTY()
	FGameplayTagContainer SprintAbilityTagsContainer;
	UPROPERTY()
	FGameplayTagContainer CrouchAbilityTagsContainer;

public:
	UStealthCharacterAbilitiesComponent();

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

private:
	UFUNCTION()
	void EndSprint() const;
	UFUNCTION()
	void UpdateTags() const;
};
