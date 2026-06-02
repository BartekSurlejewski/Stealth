#pragma once

#include "CoreMinimal.h"
#include "Interactable.generated.h"

class UGameplayAbility;
class AStealthCharacter;

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
	void PrimaryInteract(AStealthCharacter* Interactor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void SecondaryInteract(AStealthCharacter* Interactor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void SetHighlighted(bool bHighlight);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FText GetPrimaryInteractionPrompt() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	FText GetSecondaryInteractionPrompt() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	TSubclassOf<UGameplayAbility> GetPrimaryInteractionRequiredAbility() const;
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	TSubclassOf<UGameplayAbility> GetSecondaryInteractionRequiredAbility() const;

	virtual void PrimaryInteract_Implementation(AStealthCharacter* Interactor) = 0;
	virtual void SecondaryInteract_Implementation(AStealthCharacter* Interactor) = 0;
	virtual void SetHighlighted_Implementation(bool bHighlight) {};
	virtual FText GetPrimaryInteractionPrompt_Implementation() const { return FText::GetEmpty(); }
	virtual FText GetSecondaryInteractionPrompt_Implementation() const { return FText::GetEmpty(); }
	virtual TSubclassOf<UGameplayAbility> GetPrimaryInteractionRequiredAbility_Implementation() const { return nullptr; }
	virtual TSubclassOf<UGameplayAbility> GetSecondaryInteractionRequiredAbility_Implementation() const { return nullptr; }
};
