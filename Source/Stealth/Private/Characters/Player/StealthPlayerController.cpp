#include "Characters/Player/StealthPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"
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

	HUD = Cast<AStealthHUD>(GetHUD());

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	OnWidgetShownHandle = MsgSubsystem.RegisterListener<FWidgetToggleMessage>(StealthMessageChannels::TAG_Message_UI_WidgetShown, this,
	                                                                          &AStealthPlayerController::OnWidgetShown);
	OnWidgetHiddenHandle = MsgSubsystem.RegisterListener<FWidgetToggleMessage>(StealthMessageChannels::TAG_Message_UI_WidgetHidden, this,
	                                                                           &AStealthPlayerController::OnWidgetHidden);
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

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(OnWidgetShownHandle);
	MsgSubsystem.UnregisterListener(OnWidgetHiddenHandle);

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
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	const FGameplayTagMessage Message(SubmenuTag);
	MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Input_ToggleWidget, Message);
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

void AStealthPlayerController::OnWidgetShown(FGameplayTag Channel, const FWidgetToggleMessage& Message)
{
	if (HUD->IsAnyWidgetVisible(EUILayer::Menu))
	{
		UGameplayStatics::SetGamePaused(this, true);
		SetShowMouseCursor(true);
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MenuMappingContext, 1);
		}
	}
}

void AStealthPlayerController::OnWidgetHidden(FGameplayTag Channel, const FWidgetToggleMessage& Message)
{
	if (!HUD->IsAnyWidgetVisible(EUILayer::Menu))
	{
		UGameplayStatics::SetGamePaused(this, false);
		SetShowMouseCursor(false);
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(MenuMappingContext);
		}
	}
}
