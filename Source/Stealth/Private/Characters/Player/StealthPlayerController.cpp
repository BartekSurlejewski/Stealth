#include "Characters/Player/StealthPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Messages/StealthMessages.h"
#include "Stealth/Stealth.h"
#include "UI/Core/StealthHUD.h"

void AStealthPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalPlayerController())
	{
		return;
	}

	StealthPlayerState = Cast<AStealthPlayerState>(PlayerState);
	if (!StealthPlayerState)
	{
		UE_LOG(LogStealth, Error, TEXT("PlayerController: Couldn't cast player state to AStealthPlayerState."));
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

	Super::EndPlay(EndPlayReason);
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

	// UI
	EIC->BindAction(ToggleInventoryAction, ETriggerEvent::Started, this, &AStealthPlayerController::OnToggleInventoryInput);
	EIC->BindAction(ToggleDetailsMenuAction, ETriggerEvent::Started, this, &AStealthPlayerController::OnToggleDetailsMenuInput);
	EIC->BindAction(ToggleJournalMenuAction, ETriggerEvent::Started, this, &AStealthPlayerController::OnToggleJournalMenuInput);
	EIC->BindAction(PrevDetailsMenuAction, ETriggerEvent::Started, this, &AStealthPlayerController::OnPrevDetailsMenuInput);
	EIC->BindAction(NextDetailsMenuAction, ETriggerEvent::Started, this, &AStealthPlayerController::OnNextDetailsMenuInput);
}

void AStealthPlayerController::ToggleDetailsMenu(const FGameplayTag SubmenuTag)
{
	bIsInMenu = !bIsInMenu;
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		//TODO: Move managing menus and cursor show/hide somewhere else
		if (bIsInMenu)
		{
			SetShowMouseCursor(true);
			Subsystem->AddMappingContext(MenuMappingContext, 1);
		}
		else
		{
			SetShowMouseCursor(false);
			Subsystem->RemoveMappingContext(MenuMappingContext);
		}
	}

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	FGameplayTag MessageTag;
	if (bIsInMenu)
	{
		MessageTag = StealthMessageChannels::TAG_Message_Input_OpenWidget.GetTag();
	}
	else
	{
		MessageTag = StealthMessageChannels::TAG_Message_Input_CloseWidget.GetTag();
	}

	const FGameplayTagMessage MessageContent(SubmenuTag);
	MsgSubsystem.BroadcastMessage(MessageTag, MessageContent);
}

void AStealthPlayerController::OnToggleInventoryInput()
{
	ToggleDetailsMenu(StealthUiTags::TAG_UI_DetailsMenu_Inventory);
}

void AStealthPlayerController::OnToggleDetailsMenuInput()
{
	ToggleDetailsMenu(StealthUiTags::TAG_UI_DetailsMenu_DailyRegimen);
}

void AStealthPlayerController::OnToggleJournalMenuInput()
{
	ToggleDetailsMenu(StealthUiTags::TAG_UI_DetailsMenu_Journal);
}

void AStealthPlayerController::OnNextDetailsMenuInput()
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	FInputMessage Message;
	MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Input_DetailsMenu_Next.GetTag(), Message);
}

void AStealthPlayerController::OnPrevDetailsMenuInput()
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	FInputMessage Message;
	MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Input_DetailsMenu_Prev.GetTag(), Message);
}
