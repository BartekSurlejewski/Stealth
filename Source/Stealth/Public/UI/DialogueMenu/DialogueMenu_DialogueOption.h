#pragma once

#include "CoreMinimal.h"
#include "UI/Core/BaseWidgets/StealthBaseWidget.h"
#include "DialogueMenu_DialogueOption.generated.h"

class UTextBlock;
class UButton;
class UDialogueOption;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueOptionClicked, int32, OptionIndex);

UCLASS()
class STEALTH_API UDialogueMenu_DialogueOption : public UStealthBaseWidget
{
	GENERATED_BODY()

	// Events
public:
	UPROPERTY(BlueprintAssignable)
	FOnDialogueOptionClicked OnDialogueOptionClicked;

	// Methods
public:
	void SetData(const UDialogueOption* DialogueOption, const int32 NewOptionIndex);

	// Properties
protected:
	UPROPERTY(BlueprintReadOnly)
	int32 OptionIndex;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> OptionButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> OptionText;
};
