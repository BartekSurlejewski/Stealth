#pragma once

#include "CoreMinimal.h"
#include "Interactable.generated.h"

class AStealthCharacter;

UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

class STEALTH_API IInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void Interact(AStealthCharacter* Interactor);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Interaction")
	void SetHighlighted(bool bHighlight);

	virtual FText GetInteractionPrompt() const = 0;

	virtual void Interact_Implementation(AStealthCharacter* Interactor) = 0;
	virtual void SetHighlighted_Implementation(bool bHighlight) {};
};
