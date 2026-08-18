#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "GameFramework/Actor.h"
#include "Inventory/Pickable.h"
#include "Pickup.generated.h"

class USphereComponent;

UCLASS(Abstract)
class STEALTH_API APickup : public AActor, public IInteractable, public IPickable
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

protected:
	UPROPERTY(EditAnywhere, Category="Pickup")
	TObjectPtr<UItemDefinition> ItemDefinition;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Pickup")
	int Quantity = 1;
	UPROPERTY(EditAnywhere, Category="Pickup")
	FText PrimaryInteractPrompt;
	UPROPERTY(EditAnywhere, Category="Pickup")
	FText SecondaryInteractPrompt;
	UPROPERTY(EditAnywhere, Category="Pickup")
	bool bIsHoldable = false;

public:
	APickup();
	//~Begin IInteractable Interface
	virtual void PrimaryInteract_Implementation(const UStealthCharacterInteractionComponent* Interactor) override;
	virtual void SecondaryInteract_Implementation(const UStealthCharacterInteractionComponent* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
	virtual FText GetPrimaryInteractionPrompt_Implementation() const override;
	virtual FText GetSecondaryInteractionPrompt_Implementation() const override;
	virtual FGameplayTag GetPrimaryInteractionRequiredAbilityTag_Implementation() const override;
	virtual FGameplayTag GetSecondaryInteractionRequiredAbilityTag_Implementation() const override;
	//~End IInteractable Interface

	//~Begin IPickable Interface
	virtual void Initialize_Implementation(UItemDefinition* NewItemDefinition, const int NewQuantity, const FText& NewInteractPrompt = FText::GetEmpty()) override;
	virtual void OnPickup_Implementation() override;
	virtual void OnDrop_Implementation() override;
	virtual UItemDefinition* GetInventoryItem_Implementation() const override;
	virtual int GetQuantity_Implementation() const override;
	//~End IPickable Interface
};
