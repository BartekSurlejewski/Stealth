#include "UI/Core/StealthHUD.h"

#include "NativeGameplayTags.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Messages/StealthMessages.h"
#include "Stealth/Stealth.h"
#include "UI/DetailsMenu.h"


void AStealthHUD::BeginPlay()
{
	Super::BeginPlay();

	VisibleWidgets.Reserve(MAX_VISIBLE_WIDGETS);
	VisibleWidgetTags.Reserve(MAX_VISIBLE_WIDGETS);
	VisibleWidgetsCountByLayer.Add(EUILayer::None, 0);
	VisibleWidgetsCountByLayer.Add(EUILayer::HUD, 0);
	VisibleWidgetsCountByLayer.Add(EUILayer::Overlay, 0);
	VisibleWidgetsCountByLayer.Add(EUILayer::Menu, 0);
	VisibleWidgetsCountByLayer.Add(EUILayer::Popup, 0);
	VisibleWidgetsCountByLayer.Add(EUILayer::Loading, 0);

	ShowMainHUD();
	CreateOptionalHUD();

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	WidgetToggleListenerHandle = MsgSubsystem.RegisterListener<FGameplayTagMessage>(StealthMessageChannels::TAG_Message_Input_ToggleWidget, this,
	                                                                                &AStealthHUD::OnWidgetToggleMessage);
}

void AStealthHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(WidgetToggleListenerHandle);

	Super::EndPlay(EndPlayReason);
}

void AStealthHUD::ShowMainHUD()
{
	for (const auto WidgetClass : MainHUDWidgetClasses)
	{
		CreateWidgetInstance(WidgetClass, true);
	}
}

void AStealthHUD::CreateOptionalHUD()
{
	for (auto WidgetClass : OptionalHUDWidgetClasses)
	{
		CreateWidgetInstance(WidgetClass, false);
	}
}

bool AStealthHUD::IsAnyWidgetVisible(EUILayer Layer) const
{
	return VisibleWidgetsCountByLayer[Layer] > 0;
}

UStealthBaseWidget* AStealthHUD::ShowWidget(const FGameplayTag& WidgetTag)
{
	UStealthBaseWidget* WidgetInstance = ExistingWidgetsByTag.FindRef(WidgetTag);
	if (!WidgetInstance)
	{
		UE_LOG(LogStealth, Warning, TEXT("AStealthHUD: No existing widget for tag %s."), *WidgetTag.ToString());
		return nullptr;
	}

	if (VisibleWidgetTags.Contains(WidgetTag))
	{
		return WidgetInstance;
	}

	VisibleWidgetTags.Add(WidgetTag);
	VisibleWidgets.Add(WidgetInstance);
	VisibleWidgetsCountByLayer[WidgetInstance->GetLayer()] = FMath::Max(0, VisibleWidgetsCountByLayer[WidgetInstance->GetLayer()] + 1);
	WidgetInstance->Show();

	return WidgetInstance;
}

void AStealthHUD::HideWidget(const FGameplayTag& WidgetTag)
{
	UStealthBaseWidget* WidgetInstance = ExistingWidgetsByTag.FindRef(WidgetTag);
	if (!WidgetInstance)
	{
		UE_LOG(LogStealth, Warning, TEXT("AStealthHUD: No existing widget for tag %s."), *WidgetTag.ToString());
		return;
	}

	if (!VisibleWidgetTags.Contains(WidgetTag))
	{
		return;
	}

	VisibleWidgetTags.Remove(WidgetTag);
	VisibleWidgets.Remove(WidgetInstance);
	VisibleWidgetsCountByLayer[WidgetInstance->GetLayer()] = FMath::Max(0, VisibleWidgetsCountByLayer[WidgetInstance->GetLayer()] - 1);
	WidgetInstance->Hide();
}

UStealthBaseWidget* AStealthHUD::CreateWidgetInstance(TSubclassOf<UStealthBaseWidget> WidgetClass, bool bShow, int32 ZOrder)
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

	UStealthBaseWidget* WidgetInstance = CreateWidget<UStealthBaseWidget>(PC, WidgetClass);
	if (!WidgetInstance || ExistingWidgetsByTag.Contains(WidgetInstance->GetWidgetGameplayTag()))
	{
		return nullptr;
	}

	ExistingWidgetsByTag.Add(WidgetInstance->GetWidgetGameplayTag(), WidgetInstance);
	WidgetInstance->AddToViewport(ZOrder);
	WidgetInstance->Hide();

	if (bShow)
	{
		ShowWidget(WidgetInstance->GetWidgetGameplayTag());
	}

	return WidgetInstance;
}

void AStealthHUD::RemoveAndClearWidget(UStealthBaseWidget* InstanceRef)
{
	if (InstanceRef)
	{
		InstanceRef->RemoveFromParent();
		ExistingWidgetsByTag.Remove(InstanceRef->GetWidgetGameplayTag());
		InstanceRef = nullptr;
	}
}

void AStealthHUD::OnWidgetToggleMessage(FGameplayTag Channel, const FGameplayTagMessage& Message)
{
	if (Message.GameplayTag.MatchesTag(StealthUiTags::TAG_UI_DetailsMenu))
	{
		if (VisibleWidgetTags.Contains(StealthUiTags::TAG_UI_DetailsMenu))
		{
			// Close details menu
			HideWidget(StealthUiTags::TAG_UI_DetailsMenu);
		}
		else
		{
			// Open details menu
			UStealthBaseWidget* Widget = ShowWidget(StealthUiTags::TAG_UI_DetailsMenu);
			if (UDetailsMenu* DetailsMenu = Cast<UDetailsMenu>(Widget))
			{
				DetailsMenu->ShowSubmenu(Message.GameplayTag);
			}
		}
	}
	else
	{
		if (VisibleWidgetTags.Contains(Message.GameplayTag))
		{
			// Close
			HideWidget(Message.GameplayTag);
		}
		else
		{
			// Open
			ShowWidget(Message.GameplayTag);
		}
	}
}

namespace StealthUiTags
{
	UE_DEFINE_GAMEPLAY_TAG(TAG_UI_DetailsMenu, "UI.DetailsMenu");
	UE_DEFINE_GAMEPLAY_TAG(TAG_UI_DetailsMenu_Inventory, "UI.DetailsMenu.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(TAG_UI_DetailsMenu_Journal, "UI.DetailsMenu.Journal");
	UE_DEFINE_GAMEPLAY_TAG(TAG_UI_DetailsMenu_DailyRegimen, "UI.DetailsMenu.DailyRegimen");
	UE_DEFINE_GAMEPLAY_TAG(TAG_UI_DialogueMenu, "UI.DialogueMenu");
	UE_DEFINE_GAMEPLAY_TAG(TAG_UI_FinishGameMenu, "UI.FinishGameMenu");
}
