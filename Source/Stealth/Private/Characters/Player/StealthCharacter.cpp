#include "Characters/Player/StealthCharacter.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/AttributeSets/StealthCharacterAttibuteSet.h"
#include "Characters/Player/Components/PlayerInteractionComponent.h"
#include "Characters/Player/Components/StealthCharacterBehaviourComponent.h"
#include "Characters/Player/Components/StealthCharacterCollisionsComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Stealth/Stealth.h"

AStealthCharacter::AStealthCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(55.0f, 96.0f);

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(false);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("BlockAll"));

	// Create the Camera Component	
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCamera->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCamera->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->bEnableFirstPersonFieldOfView = true;
	FirstPersonCamera->bEnableFirstPersonScale = true;
	FirstPersonCamera->FirstPersonFieldOfView = 70.0f;
	FirstPersonCamera->FirstPersonScale = 0.6f;

	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::None;
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->SetCastShadow(true);
	GetMesh()->SetCastHiddenShadow(true);
	GetMesh()->bCastDynamicShadow = true;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	StealthCharacterAbilitiesComponent = CreateDefaultSubobject<UStealthCharacterBehaviourComponent>("Stealth Character Behaviour Component");
	InteractionComponent = CreateDefaultSubobject<UPlayerInteractionComponent>(TEXT("Interaction Component"));
	CollisionsComponent = CreateDefaultSubobject<UStealthCharacterCollisionsComponent>(TEXT("Collisions Component"));
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimuli Source Component"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("Ability System Component"));
	AttributeSet = CreateDefaultSubobject<UStealthCharacterAttributeSet>("Attribute Set");
}


void AStealthCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AStealthCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (CollisionsComponent)
	{
		CollisionsComponent->OnBeginOverlap(OtherActor);
	}
}

void AStealthCharacter::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (CollisionsComponent)
	{
		CollisionsComponent->OnEndOverlap(OtherActor);
	}
}

//~Begin Input
void AStealthCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AStealthCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AStealthCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AStealthCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AStealthCharacter::DoInteract()
{
	if (!InteractionComponent)
	{
		return;
	}

	InteractionComponent->Interact();
}

//~End Input


void AStealthCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AStealthCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AStealthCharacter::LookInput);

		// Interaction
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AStealthCharacter::DoInteract);
		InteractionComponent->SetInteractInputAction(InteractAction);

		// Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, StealthCharacterAbilitiesComponent.Get(),
		                                   &UStealthCharacterBehaviourComponent::DoSprintInputStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, StealthCharacterAbilitiesComponent.Get(),
		                                   &UStealthCharacterBehaviourComponent::DoSprintInputEnd);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, StealthCharacterAbilitiesComponent.Get(),
		                                   &UStealthCharacterBehaviourComponent::DoCrouchInputStart);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, StealthCharacterAbilitiesComponent.Get(),
		                                   &UStealthCharacterBehaviourComponent::DoCrouchInputEnd);

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, StealthCharacterAbilitiesComponent.Get(), &UStealthCharacterBehaviourComponent::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, StealthCharacterAbilitiesComponent.Get(), &UStealthCharacterBehaviourComponent::DoJumpEnd);
	}
	else
	{
		UE_LOG(LogStealth, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
}

UAbilitySystemComponent* AStealthCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
