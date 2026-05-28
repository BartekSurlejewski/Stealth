#include "Characters/Player/StealthPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/Pickable.h"
#include "Inventory/Items/InventoryItem.h"
#include "Inventory/Items/ItemDefinition.h"
#include "Stealth/Stealth.h"

AStealthPlayerController::AStealthPlayerController()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory Component");
}

void AStealthPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController())
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get Enhanced Input Subsystem!"));
	}

	InventoryComponent->OnItemRemoved.AddDynamic(this, &AStealthPlayerController::InventoryComponent_OnItemRemoved);
}

void AStealthPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
		}
	}

	InventoryComponent->OnItemRemoved.RemoveAll(this);

	Super::EndPlay(EndPlayReason);
}

void AStealthPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AStealthPlayerController::OnUnPossess()
{
	// Clean up mappings when unpossessing
}

void AStealthPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	BindInputActions();
}

void AStealthPlayerController::BindInputActions()
{
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EIC)
	{
		return;
	}

	// Inventory
	EIC->BindAction(InventoryAction, ETriggerEvent::Started, this, &AStealthPlayerController::OnOpenInventoryInput);
}

void AStealthPlayerController::OnOpenInventoryInput()
{
	bIsInMenu = !bIsInMenu;
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		//TODO: Add some manager/subsystem for managing UI menus
		if (bIsInMenu)
		{
			SetShowMouseCursor(true);
			Subsystem->RemoveMappingContext(DefaultMappingContext);
			Subsystem->AddMappingContext(MenuMappingContext, 0);
		}
		else
		{
			SetShowMouseCursor(false);
			Subsystem->RemoveMappingContext(MenuMappingContext);
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	OnInventoryInput.Broadcast();

	UE_LOG(LogStealth, Log, TEXT("[Inventory]"))
	for (const auto Item : InventoryComponent->GetItems())
	{
		UE_LOG(LogStealth, Log, TEXT("Item: %s - %i"), *Item.ItemDefinition->Name.ToString(), Item.Quantity)
	}
}

void AStealthPlayerController::InventoryComponent_OnItemRemoved(const FInventoryItem& Item, int Quantity, bool bShouldDrop)
{
	if (bShouldDrop)
	{
		TSubclassOf<AActor> ActorToDropClass = Item.ItemDefinition->PickupActorClass;
		if (!ActorToDropClass)
		{
			return;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		const FVector PickableSpawnLocation = GetPawn()->GetActorLocation() + GetPawn()->GetActorForwardVector() * 100.f;
		const FRotator PickableSpawnRotation = FRotator::ZeroRotator;

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ActorToDropClass, PickableSpawnLocation, PickableSpawnRotation, SpawnParams);
		if (!SpawnedActor)
		{
			return;
		}
		if (SpawnedActor->Implements<UPickable>())
		{
			IPickable::Execute_Initialize(SpawnedActor, Item.ItemDefinition, Item.Quantity, Item.ItemDefinition->Name);
		}
	}
}
