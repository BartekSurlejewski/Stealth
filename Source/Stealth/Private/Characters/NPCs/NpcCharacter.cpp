#include "Characters/NPCs/NpcCharacter.h"

#include "Characters/NPCs/NpcAiController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ANpcCharacter::ANpcCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(55.0f, 96.0f);

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void ANpcCharacter::BeginPlay()
{
	Super::BeginPlay();

	AiController = Cast<ANpcAiController>(GetController());
}

void ANpcCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ANpcCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ANpcCharacter::PrimaryInteract_Implementation(AStealthCharacter* Interactor) {}

void ANpcCharacter::SetHighlighted_Implementation(bool bHighlight)
{
	IInteractable::SetHighlighted_Implementation(bHighlight);
}

FText ANpcCharacter::GetPrimaryInteractionPrompt_Implementation() const
{
	return FText::FromString("Talk");
}

FGameplayTag ANpcCharacter::GetPrimaryInteractionAbilityTag_Implementation() const
{
	return IInteractable::GetPrimaryInteractionAbilityTag_Implementation();
}
