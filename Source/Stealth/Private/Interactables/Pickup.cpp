#include "Environment/Interactables/Pickup.h"

#include "Components/StaticMeshComponent.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = false;

	// create the root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// create the mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_PhysicsBody);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block); // collide with world geometry
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

void APickup::PrimaryInteract_Implementation(AStealthPlayerCharacter* Interactor)
{
	// hide this mesh
	SetActorHiddenInGame(true);
	// disable collision
	SetActorEnableCollision(false);
	// disable ticking
	SetActorTickEnabled(false);
}

void APickup::SecondaryInteract_Implementation(AStealthPlayerCharacter* Interactor) {}

void APickup::SetHighlighted_Implementation(bool bHighlight)
{
	IInteractable::SetHighlighted_Implementation(bHighlight);
}

FText APickup::GetPrimaryInteractionPrompt_Implementation() const
{
	const FText& BaseName = InteractPrompt.IsEmpty() ? ItemDefinition->Name : InteractPrompt;

	if (Quantity > 1)
	{
		return FText::Format(NSLOCTEXT("Pickup", "PromptWithQuantity", "{0} x{1}"), BaseName, FText::AsNumber(Quantity));
	}

	return BaseName;
}

FText APickup::GetSecondaryInteractionPrompt_Implementation() const
{
	return IInteractable::GetSecondaryInteractionPrompt_Implementation();
}

FGameplayTag APickup::GetPrimaryInteractionRequiredAbilityTag_Implementation() const
{
	return IInteractable::GetPrimaryInteractionRequiredAbilityTag_Implementation();
}

FGameplayTag APickup::GetSecondaryInteractionRequiredAbilityTag_Implementation() const
{
	return IInteractable::GetSecondaryInteractionRequiredAbilityTag_Implementation();
}

void APickup::Initialize_Implementation(UItemDefinition* NewItemDefinition, const int NewQuantity, const FText& NewInteractPrompt)
{
	this->ItemDefinition = NewItemDefinition;
	this->Quantity = NewQuantity;
	this->InteractPrompt = NewInteractPrompt;
}

UItemDefinition* APickup::GetInventoryItem_Implementation() const
{
	return ItemDefinition;
}

int APickup::GetQuantity_Implementation() const
{
	return Quantity;
}
