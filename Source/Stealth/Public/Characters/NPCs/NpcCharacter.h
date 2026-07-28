#pragma once

#include "CoreMinimal.h"
#include "Environment/Interactables/Interactable.h"
#include "GameFramework/Character.h"
#include "NpcCharacter.generated.h"

class UDialogueComponent;
class ANpcAiController;

UCLASS(Abstract)
class STEALTH_API ANpcCharacter : public ACharacter, public IInteractable
{
	GENERATED_BODY()

	/*Methods*/
public:
	ANpcCharacter();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
#if WITH_EDITOR
	virtual void PostEditImport() override;
#endif

	void AssignGuidFromSave(const FGuid& SavedGuid) { NpcGUID = SavedGuid; }
	UFUNCTION(BlueprintPure, Category = "NPC|Identification")
	const FGuid& GetNpcGUID() const { return NpcGUID; }

	//~Begin IInteractable Interface
	virtual void PrimaryInteract_Implementation(AStealthCharacter* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
	virtual FText GetPrimaryInteractionPrompt_Implementation() const override;
	virtual FGameplayTag GetPrimaryInteractionAbilityTag_Implementation() const override;
	//~End IInteractable Interface

	/*Properties*/
protected:
	UPROPERTY();
	TObjectPtr<ANpcAiController> AiController;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UDialogueComponent> DialogueComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Identification")
	FGuid NpcGUID;
};
