#include "UI/Core/BaseWidgets/StealthBaseWidget.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/StealthMessages.h"

void UStealthBaseWidget::Show()
{
	SetVisibility(ESlateVisibility::Visible);
	OnShow();

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	FWidgetToggleMessage Message;
	Message.WidgetLayer = GetLayer();
	MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_UI_WidgetShown.GetTag(), Message);
}

void UStealthBaseWidget::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
	OnHide();

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	FWidgetToggleMessage Message;
	Message.WidgetLayer = GetLayer();
	MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_UI_WidgetHidden.GetTag(), Message);
}
