#include "Interactables/Pickup.h"

#include "Characters/Player/StealthPlayerCharacter.h"
#include "Characters/Player/Components/StealthCharacterInteractionComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Stealth/Stealth.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// create the mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionObjectType(ECC_PhysicsBody);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block); // collide with world geometry
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	Mesh->SetMobility(EComponentMobility::Movable);

	RootComponent = Mesh;
}

void APickup::PrimaryInteract_Implementation(const UStealthCharacterInteractionComponent* Interactor)
{
	// hide this mesh
	SetActorHiddenInGame(true);
	// disable collision
	SetActorEnableCollision(false);
	// disable ticking
	SetActorTickEnabled(false);
}

void APickup::SecondaryInteract_Implementation(const UStealthCharacterInteractionComponent* Interactor)
{
	UE_LOG(LogStealth, Warning, TEXT("[Pickup] Secondary interaction"));

	if (bIsHoldable)
	{
		Interactor->TryPickupItem(this);
	}

	UE_LOG(LogStealth, Warning, TEXT("[Pickup] SimPhysics=%d AbsLoc=%d Parent=%s"),
	       Mesh->IsSimulatingPhysics(),
	       Mesh->IsUsingAbsoluteLocation(),
	       *GetNameSafe(GetAttachParentActor()));
}

void APickup::SetHighlighted_Implementation(bool bHighlight)
{
	IInteractable::SetHighlighted_Implementation(bHighlight);
}

FText APickup::GetPrimaryInteractionPrompt_Implementation() const
{
	const FText& BaseName = PrimaryInteractPrompt.IsEmpty() ? ItemDefinition->Name : PrimaryInteractPrompt;

	if (Quantity > 1)
	{
		return FText::Format(NSLOCTEXT("Pickup", "PromptWithQuantity", "{0} x{1}"), BaseName, FText::AsNumber(Quantity));
	}

	return BaseName;
}

FText APickup::GetSecondaryInteractionPrompt_Implementation() const
{
	return SecondaryInteractPrompt;
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
	this->PrimaryInteractPrompt = NewInteractPrompt;
}

void APickup::OnPickup_Implementation()
{
	UE_LOG(LogStealth, Warning, TEXT("[Pickup] On Pickup"));

	Mesh->SetSimulatePhysics(false);
	Mesh->SetUsingAbsoluteLocation(false);
	Mesh->SetUsingAbsoluteRotation(false);
	Mesh->SetUsingAbsoluteScale(false);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Fix for mesh being detached when disabling physics
	RootComponent = Mesh;
}

void APickup::OnDrop_Implementation()
{
	Mesh->SetSimulatePhysics(true);
	Mesh->SetUsingAbsoluteLocation(true);
	Mesh->SetUsingAbsoluteRotation(true);
	Mesh->SetUsingAbsoluteScale(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	RootComponent = Mesh;
}

UItemDefinition* APickup::GetInventoryItem_Implementation() const
{
	return ItemDefinition;
}

int APickup::GetQuantity_Implementation() const
{
	return Quantity;
}
