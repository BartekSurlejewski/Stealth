#include "Characters/Player/StealthPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/Items/InventoryItem.h"
#include "Inventory/Items/ItemDefinition.h"
#include "Stealth/Stealth.h"

AStealthPlayerController::AStealthPlayerController()
{
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory Component");
}

void AStealthPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

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
}

void AStealthPlayerController::OnUnPossess()
{
	// Clean up mappings when unpossessing
	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(DefaultMappingContext);
		}
	}

	Super::OnUnPossess();
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
		UE_LOG(LogStealth, Log, TEXT("Item: %s - %i"), *Item.ItemDefinition->Name.ToString(), Item.Amount)
	}
}
