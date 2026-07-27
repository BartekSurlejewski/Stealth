#pragma once

#include "CoreMinimal.h"
#include "Core/StealthWorldSubsystem.h"
#include "DialogueManagerSubsystem.generated.h"

class UDialogueController;
class UDialogueComponent;

UCLASS()
class STEALTH_API UDialogueManagerSubsystem : public UStealthWorldSubsystem
{
	GENERATED_BODY()

public:
	static UDialogueManagerSubsystem* Get(const UObject* WorldContextObject)
	{
		return UStealthWorldSubsystem::Get<UDialogueManagerSubsystem>(WorldContextObject);
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void OnWorldEndPlay(UWorld& InWorld) override;

	UFUNCTION(BlueprintCallable)
	bool TryStartDialogue(UDialogueComponent* NpcDialogueComponent);

	/*Properties*/
private:
	UPROPERTY()
	TObjectPtr<UDialogueController> DialogueController;
};
