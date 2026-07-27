#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DialogueOption.generated.h"

class USpecialDialogueBehaviour;

UCLASS(EditInlineNew, DefaultToInstanced, Blueprintable)
class STEALTH_API UDialogueOption : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText OptionText;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ResponseText;
	UPROPERTY(EditAnywhere, Instanced)
	TArray<TObjectPtr<USpecialDialogueBehaviour>> CustomActions;
	UPROPERTY(EditDefaultsOnly)
	bool bCanBeUsedOnce = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUsedAlready = false;
};
