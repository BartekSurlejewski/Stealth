#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UI/Core/BaseWidgets/StealthBaseMenu.h"
#include "UI/Core/BaseWidgets/StealthBaseWidget.h"
#include "DialogueMenu.generated.h"

struct FDialogueMessage;
class UVerticalBox;
class UTextBlock;
class UDialogueOption;

UCLASS()
class STEALTH_API UDialogueMenu : public UStealthBaseMenu
{
	GENERATED_BODY()
	// Methods
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	UFUNCTION()
	void ShowDialogueOptions(const TArray<UDialogueOption*> NewDialogueOptions);
	UFUNCTION()
	void HideDialogueMenu();

protected:
	// Listen for dialogue start/end
	UFUNCTION()
	void OnDialogueStarted(FGameplayTag Channel, const FDialogueMessage& Message);
	UFUNCTION()
	void OnDialogueEnded(FGameplayTag Channel, const FDialogueMessage& Message);
	// Button click handler
	UFUNCTION()
	void OnOptionButtonClicked();

	/*Properties*/
protected:
	// Bind these in widget designer
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NpcNameText;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NPCGreetingText;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> OptionsContainer;
	// Keep reference to options for selection
	UPROPERTY(Transient)
	TArray<TObjectPtr<UDialogueOption>> CurrentDialogueOptions;

	FGameplayMessageListenerHandle DialogueStartedListenerHandle;
	FGameplayMessageListenerHandle DialogueEndedListenerHandle;
};
