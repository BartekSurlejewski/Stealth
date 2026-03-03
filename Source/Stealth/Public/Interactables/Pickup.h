// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

class USphereComponent;

UCLASS(Abstract)
class STEALTH_API APickup : public AActor, public IInteractable
{
	GENERATED_BODY()
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> Mesh;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> SphereCollision;
	UPROPERTY(EditAnywhere, Category="Pickup")
	FText InteractPrompt;

public:
	// Sets default values for this actor's properties
	APickup();

	//~Begin IInteractable Interface
	virtual void Interact_Implementation(AStealthCharacter* Interactor) override;
	virtual void SetHighlighted_Implementation(bool bHighlight) override;
	virtual FText GetInteractionPrompt() const override;
	//~End IInteractable Interface
};
