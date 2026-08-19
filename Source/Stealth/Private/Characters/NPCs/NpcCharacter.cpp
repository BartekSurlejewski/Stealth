#include "Characters/NPCs/NpcCharacter.h"

#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/AI/Schedule/NpcScheduleComponent.h"
#include "Components/CapsuleComponent.h"
#include "DialogueSystem/DialogueComponent.h"
#include "DialogueSystem/DialogueManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

ANpcCharacter::ANpcCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickInterval = 0.2f;

	GetCapsuleComponent()->InitCapsuleSize(55.0f, 96.0f);
	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	DialogueComponent = CreateDefaultSubobject<UDialogueComponent>("Dialogue Component");

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		NpcGUID = FGuid::NewGuid();
	}
}

void ANpcCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (!NpcGUID.IsValid())
	{
		NpcGUID = FGuid::NewGuid();
		MarkPackageDirty();
	}
#endif
}

void ANpcCharacter::BeginPlay()
{
	Super::BeginPlay();

	NpcRegistrySubsystem = UCharactersRegistrySubsystem::Get(this);
	ANpcAiController* AiController = Cast<ANpcAiController>(GetController());
	NpcRegistrySubsystem->RegisterNpc(this, AiController);
}

void ANpcCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	NpcRegistrySubsystem->UnregisterNpc(NpcGUID);

	Super::EndPlay(EndPlayReason);
}

void ANpcCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

#if WITH_EDITOR
void ANpcCharacter::PostEditImport()
{
	Super::PostEditImport();

	NpcGUID = FGuid::NewGuid();
	MarkPackageDirty();
}
#endif

void ANpcCharacter::PrimaryInteract_Implementation(const UStealthCharacterInteractionComponent* Interactor)
{
	auto DialogueManager = UDialogueManagerSubsystem::Get(this);
	if (!DialogueManager || !DialogueComponent)
	{
		return;
	}

	DialogueManager->TryStartDialogue(DialogueComponent);
}

void ANpcCharacter::SetHighlighted_Implementation(bool bHighlight)
{
	IInteractable::SetHighlighted_Implementation(bHighlight);
}

FText ANpcCharacter::GetPrimaryInteractionPrompt_Implementation() const
{
	return FText::FromString("Talk");
}

FGameplayTag ANpcCharacter::GetPrimaryInteractionRequiredAbilityTag_Implementation() const
{
	return IInteractable::GetPrimaryInteractionRequiredAbilityTag_Implementation();
}
