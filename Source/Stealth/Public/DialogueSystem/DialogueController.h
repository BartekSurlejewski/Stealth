#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "UObject/Object.h"
#include "DialogueController.generated.h"

struct FDialogueMessage;
struct FIntMessage;
struct FGameplayTag;
class UDialogueComponent;

UCLASS()
class STEALTH_API UDialogueController : public UObject
{
	GENERATED_BODY()
	// Events
	// Methods
public:
	UFUNCTION()
	void Initialize();
	UFUNCTION()
	void Dispose() const;
	UFUNCTION()
	bool TryStartDialogue(UDialogueComponent* NewDialogueComponent);

private:
	UFUNCTION()
	void Input_OnDialogueOptionChosen(FGameplayTag Channel, const FIntMessage& Message);
	UFUNCTION()
	void Input_OnDialogueEnd(FGameplayTag Channel, const FDialogueMessage& Message);

	// Properties
private:
	UPROPERTY()
	TObjectPtr<UDialogueComponent> NpcDialogueComponent;
	UPROPERTY()
	bool bIsDialogueOngoing = false;

	// Listener handles
	FGameplayMessageListenerHandle DialogueOptionChosenMessageListenerHandle;
	FGameplayMessageListenerHandle DialogueEndMessageListenerHandle;
};
