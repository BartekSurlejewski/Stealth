#pragma once

#include "CoreMinimal.h"
#include "UI/Core/StealthUserWidget.h"
#include "DialogueMenu.generated.h"

class UDialogueOption;

UCLASS()
class STEALTH_API UDialogueMenu : public UStealthUserWidget
{
	GENERATED_BODY()
	// Methods
public:
	UFUNCTION()
	void ShowDialogueOptions(const TArray<UDialogueOption*> NewDialogueOptions);
};
