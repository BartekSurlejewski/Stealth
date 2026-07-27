#include "UI/Core/StealthHUD.h"

#include "NativeGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/StealthMessages.h"
#include "Stealth/Stealth.h"
#include "UI/DetailsMenu.h"
#include "UI/Core/StealthUserWidget.h"


void AStealthHUD::BeginPlay()
{
	Super::BeginPlay();

	ShowMainHUD();
	CreateOptionalHUD();

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	WidgetOpenListenerHandle = MsgSubsystem.RegisterListener<FGameplayTagMessage>(StealthMessageChannels::TAG_Message_Input_OpenWidget, this,
	                                                                              &AStealthHUD::OnWidgetOpenMessage);
	WidgetCloseListenerHandle = MsgSubsystem.RegisterListener<FGameplayTagMessage>(StealthMessageChannels::TAG_Message_Input_CloseWidget, this,
	                                                                               &AStealthHUD::OnWidgetCloseMessage);
}

void AStealthHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(WidgetOpenListenerHandle);
	MsgSubsystem.UnregisterListener(WidgetCloseListenerHandle);

	Super::EndPlay(EndPlayReason);
}

void AStealthHUD::ShowMainHUD()
{
	for (auto WidgetClass : MainHUDWidgetClasses)
	{
		CreateAndShowWidget(WidgetClass, true);
	}
}

void AStealthHUD::CreateOptionalHUD()
{
	for (auto WidgetClass : OptionalHUDWidgetClasses)
	{
		CreateAndShowWidget(WidgetClass, false);
	}
}

UStealthUserWidget* AStealthHUD::ShowWidget(FGameplayTag WidgetTag, int32 ZOrder)
{
	UStealthUserWidget* WidgetInstance = ExistingWidgetsByTag.FindRef(WidgetTag);
	if (!WidgetInstance)
	{
		UE_LOG(LogStealth, Warning, TEXT("AStealthHUD: No existing widget for tag %s."), *WidgetTag.ToString());
		return nullptr;
	}

	WidgetInstance->AddToViewport(ZOrder);
	WidgetInstance->OnShow();
	return WidgetInstance;
}

void AStealthHUD::HideWidget(FGameplayTag WidgetTag)
{
	UStealthUserWidget* WidgetInstance = ExistingWidgetsByTag.FindRef(WidgetTag);
	if (!WidgetInstance)
	{
		UE_LOG(LogStealth, Warning, TEXT("AStealthHUD: No existing widget for tag %s."), *WidgetTag.ToString());
		return;
	}

	WidgetInstance->RemoveFromParent();
	WidgetInstance->OnHide();
}

UStealthUserWidget* AStealthHUD::CreateAndShowWidget(TSubclassOf<UStealthUserWidget> WidgetClass, bool bShow, int32 ZOrder)
{
	if (!WidgetClass)
	{
		UE_LOG(LogStealth, Warning, TEXT("AStealthHUD: No widget class assigned."));
		return nullptr;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return nullptr;
	}

	UStealthUserWidget* WidgetInstance = CreateWidget<UStealthUserWidget>(PC, WidgetClass);
	if (!WidgetInstance || ExistingWidgetsByTag.Contains(WidgetInstance->GetWidgetGameplayTag()))
	{
		return nullptr;
	}

	ExistingWidgetsByTag.Add(WidgetInstance->GetWidgetGameplayTag(), WidgetInstance);

	if (bShow)
	{
		ShowWidget(WidgetInstance->GetWidgetGameplayTag(), ZOrder);
	}

	return WidgetInstance;
}

void AStealthHUD::RemoveAndClearWidget(UStealthUserWidget* InstanceRef)
{
	if (InstanceRef)
	{
		InstanceRef->RemoveFromParent();
		ExistingWidgetsByTag.Remove(InstanceRef->GetWidgetGameplayTag());
		InstanceRef = nullptr;
	}
}

void AStealthHUD::OnWidgetOpenMessage(FGameplayTag Channel, const FGameplayTagMessage& Message)
{
	if (Message.GameplayTag.MatchesTag(StealthUiTags::TAG_UI_DetailsMenu))
	{
		UStealthUserWidget* Widget = ShowWidget(StealthUiTags::TAG_UI_DetailsMenu);
		UDetailsMenu* DetailsMenu = Cast<UDetailsMenu>(Widget);
		if (DetailsMenu)
		{
			DetailsMenu->Show(Message.GameplayTag);
		}
	}
	else
	{
		ShowWidget(Message.GameplayTag);
	}
}

void AStealthHUD::OnWidgetCloseMessage(FGameplayTag Channel, const FGameplayTagMessage& Message)
{
	if (Message.GameplayTag.MatchesTag(StealthUiTags::TAG_UI_DetailsMenu))
	{
		HideWidget(StealthUiTags::TAG_UI_DetailsMenu);
	}
	else
	{
		HideWidget(Message.GameplayTag);
	}
}

namespace StealthUiTags
{
	UE_DEFINE_GAMEPLAY_TAG(TAG_UI_DetailsMenu, "UI.DetailsMenu");
	UE_DEFINE_GAMEPLAY_TAG(TAG_UI_DetailsMenu_Inventory, "UI.DetailsMenu.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(TAG_UI_DetailsMenu_Journal, "UI.DetailsMenu.Journal");
	UE_DEFINE_GAMEPLAY_TAG(TAG_UI_DetailsMenu_DailyRegimen, "UI.DetailsMenu.DailyRegimen");
}
