#include "Interactables/Pickup.h"

#include "Components/SphereComponent.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;

	// create the root
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	// create the collision sphere
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision"));
	SphereCollision->SetupAttachment(RootComponent);
	SphereCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECC_WorldStatic);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	SphereCollision->bFillCollisionUnderneathForNavmesh = true;

	// create the mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);

	Mesh->SetCollisionProfileName(FName("NoCollision"));
}

void APickup::Interact_Implementation(AStealthCharacter* Interactor)
{
	// hide this mesh
	SetActorHiddenInGame(true);
	// disable collision
	SetActorEnableCollision(false);
	// disable ticking
	SetActorTickEnabled(false);
}

void APickup::SetHighlighted_Implementation(bool bHighlight)
{
	IInteractable::SetHighlighted_Implementation(bHighlight);
}

FText APickup::GetInteractionPrompt_Implementation() const
{
	return InteractPrompt;
}
