#include "Characters/Player/StealthPlayerCharacter.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/AttributeSets/StealthCharacterAttibuteSet.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/Player/Components/StealthCharacterInteractionComponent.h"
#include "Characters/Player/Components/StealthCharacterAbilitiesComponent.h"
#include "Characters/Player/Components/StealthCharacterCollisionsComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Stealth/Stealth.h"

AStealthPlayerCharacter::AStealthPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(55.0f, 96.0f);
	RootComponent = GetCapsuleComponent();

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(false);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("BlockAll"));
	FirstPersonMesh->SetCollisionObjectType(ECC_Pawn);

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
	GetMesh()->SetCollisionObjectType(ECC_Pawn);

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;

	InteractionComponent = CreateDefaultSubobject<UStealthCharacterInteractionComponent>(TEXT("Interaction Component"));
	CollisionsComponent = CreateDefaultSubobject<UStealthCharacterCollisionsComponent>(TEXT("Collisions Component"));
	StimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimuli Source Component"));
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("Ability System Component"));
	AttributeSet = CreateDefaultSubobject<UStealthCharacterAttributeSet>(TEXT("Attribute Set"));
	StealthCharacterAbilitiesComponent = CreateDefaultSubobject<UStealthCharacterAbilitiesComponent>(TEXT("Stealth Character Abilities Component"));
}

void AStealthPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UCharactersRegistrySubsystem* CharactersRegistry = UCharactersRegistrySubsystem::Get(this))
	{
		CharactersRegistry->RegisterPlayerCharacter(this);
	}
}

void AStealthPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UCharactersRegistrySubsystem* CharactersRegistry = UCharactersRegistrySubsystem::Get(this))
	{
		CharactersRegistry->UnregisterPlayerCharacter(this);
	}
}

void AStealthPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void AStealthPlayerCharacter::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (CollisionsComponent)
	{
		CollisionsComponent->OnBeginOverlap(OtherActor);
	}
}

void AStealthPlayerCharacter::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (CollisionsComponent)
	{
		CollisionsComponent->OnEndOverlap(OtherActor);
	}
}

AActor* AStealthPlayerCharacter::TryDropItem(const TSubclassOf<AActor> ItemToDropClass) const
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	const FVector PickableSpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.f;
	const FRotator PickableSpawnRotation = FRotator::ZeroRotator;

	return GetWorld()->SpawnActor<AActor>(ItemToDropClass, PickableSpawnLocation, PickableSpawnRotation, SpawnParams);
}

//~Begin Input
void AStealthPlayerCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AStealthPlayerCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);
}

void AStealthPlayerCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AStealthPlayerCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AStealthPlayerCharacter::DoPrimaryInteract()
{
	if (!InteractionComponent)
	{
		return;
	}

	InteractionComponent->Interact(true);
}

void AStealthPlayerCharacter::DoSecondaryInteract()
{
	UE_LOG(LogStealth, Warning, TEXT("Secondary Interact"));
	if (!InteractionComponent)
	{
		return;
	}

	InteractionComponent->Interact(false);
}

void AStealthPlayerCharacter::DoThrow()
{
	UE_LOG(LogStealth, Warning, TEXT("Throw Input"));
	if (!InteractionComponent)
	{
		return;
	}

	InteractionComponent->ThrowPickedItem();
}

//~End Input

void AStealthPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AStealthPlayerCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AStealthPlayerCharacter::LookInput);

		// Interaction
		EnhancedInputComponent->BindAction(PrimaryInteractAction, ETriggerEvent::Started, this, &AStealthPlayerCharacter::DoPrimaryInteract);
		EnhancedInputComponent->BindAction(SecondaryInteractAction, ETriggerEvent::Started, this, &AStealthPlayerCharacter::DoSecondaryInteract);

		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Started, this, &AStealthPlayerCharacter::DoThrow);

		if (InteractionComponent)
		{
			InteractionComponent->InitializeInput(PrimaryInteractAction, SecondaryInteractAction);
		}

		// // Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, StealthCharacterAbilitiesComponent.Get(),
		                                   &UStealthCharacterAbilitiesComponent::DoSprintInputStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, StealthCharacterAbilitiesComponent.Get(),
		                                   &UStealthCharacterAbilitiesComponent::DoSprintInputEnd);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, StealthCharacterAbilitiesComponent.Get(),
		                                   &UStealthCharacterAbilitiesComponent::DoCrouchInputStart);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, StealthCharacterAbilitiesComponent.Get(),
		                                   &UStealthCharacterAbilitiesComponent::DoCrouchInputEnd);

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, StealthCharacterAbilitiesComponent.Get(), &UStealthCharacterAbilitiesComponent::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, StealthCharacterAbilitiesComponent.Get(), &UStealthCharacterAbilitiesComponent::DoJumpEnd);
	}
	else
	{
		UE_LOG(LogStealth, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
}

UAbilitySystemComponent* AStealthPlayerCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
