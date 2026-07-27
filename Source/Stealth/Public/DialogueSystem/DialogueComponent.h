#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "DialogueComponent.generated.h"


class USpecialDialogueBehaviour;
class ACharacter;

UCLASS(Blueprintable)
class STEALTH_API UDialogueOption : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText OptionText;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ResponseText;
	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<USpecialDialogueBehaviour>> CustomActions;
	UPROPERTY(EditDefaultsOnly)
	bool bCanBeUsedOnce = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsedAlready = false;
};

UCLASS(ClassGroup=(Dialogue), meta=(BlueprintSpawnableComponent))
class STEALTH_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()

	/*Methods*/
public:
	UDialogueComponent();

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	UDialogueOption* GetDialogueOption(int32 OptionIndex);

	/*Properties*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<TObjectPtr<UDialogueOption>> DialogueOptions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText NPCGreeting = FText::FromString("Hello prisoner");
};
