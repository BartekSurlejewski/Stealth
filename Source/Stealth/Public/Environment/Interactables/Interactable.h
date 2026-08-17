#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

class UGameplayAbility;
class AStealthPlayerCharacter;

UINTERFACE(MinimalAPI, BlueprintType)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

class STEALTH_API IInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void PrimaryInteract(AStealthPlayerCharacter* Interactor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void SecondaryInteract(AStealthPlayerCharacter* Interactor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void SetHighlighted(bool bHighlight);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FText GetPrimaryInteractionPrompt() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FText GetSecondaryInteractionPrompt() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FGameplayTag GetPrimaryInteractionRequiredAbilityTag() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FGameplayTag GetSecondaryInteractionRequiredAbilityTag() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FGameplayTag GetPrimaryInteractionMessageTag() const;

	virtual void PrimaryInteract_Implementation(AStealthPlayerCharacter* Interactor) {}
	virtual void SecondaryInteract_Implementation(AStealthPlayerCharacter* Interactor) {}
	virtual void SetHighlighted_Implementation(bool bHighlight) {}
	virtual FText GetPrimaryInteractionPrompt_Implementation() const { return FText::GetEmpty(); }
	virtual FText GetSecondaryInteractionPrompt_Implementation() const { return FText::GetEmpty(); }
	virtual FGameplayTag GetPrimaryInteractionRequiredAbilityTag_Implementation() const { return FGameplayTag(); }
	virtual FGameplayTag GetSecondaryInteractionRequiredAbilityTag_Implementation() const { return FGameplayTag(); }
};
