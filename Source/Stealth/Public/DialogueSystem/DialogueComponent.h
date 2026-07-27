#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueComponent.generated.h"

class UDialogueOption;
class USpecialDialogueBehaviour;
class ACharacter;

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
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category="Dialogue")
	TArray<TObjectPtr<UDialogueOption>> DialogueOptions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText NPCGreeting = FText::FromString("Hello prisoner");
};
