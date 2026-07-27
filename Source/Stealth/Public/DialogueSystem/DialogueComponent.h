#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "DialogueComponent.generated.h"


class ACharacter;

USTRUCT(BlueprintType)
struct FDialogueOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText OptionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ResponseText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag QuestToGrant; // NEW: granting quests via dialogue
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueOptionSelected, FGameplayTag, GrantedQuestTag);

UCLASS(ClassGroup=(Dialogue), meta=(BlueprintSpawnableComponent))
class STEALTH_API UDialogueComponent : public UActorComponent
{
	GENERATED_BODY()
	/*Events*/
public:
	UPROPERTY(BlueprintAssignable)
	FOnDialogueOptionSelected OnDialogueOptionSelected;

	/*Methods*/
public:
	UDialogueComponent();

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void SelectDialogueOption(int32 OptionIndex, ACharacter* PlayerCharacter);

	/*Properties*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	TArray<FDialogueOption> DialogueOptions;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialogue")
	FText NPCGreeting = FText::FromString("Hello prisoner");
};
