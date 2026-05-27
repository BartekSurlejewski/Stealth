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
	int Amount = 1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> SphereCollision;
	UPROPERTY(EditAnywhere, Category="Pickup")
	FText InteractPrompt;

public:
	APickup();
	void Initialize(UItemDefinition& NewItemDefinition, const int NewAmount);

	//~Begin IInteractable Interface
	virtual void Interact_Implementation(AStealthCharacter* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
	virtual FText GetInteractionPrompt_Implementation() const override;
	//~End IInteractable Interface

	//~Begin IPickable Interface
	virtual UItemDefinition* GetInventoryItem_Implementation() const override;
	virtual int GetAmount_Implementation() const override;
	//~End IPickable Interface
};
