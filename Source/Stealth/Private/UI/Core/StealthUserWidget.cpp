#include "UI/Core/StealthUserWidget.h"

void UStealthUserWidget::Show()
{
	SetVisibility(ESlateVisibility::Visible);
	OnShow();
}

void UStealthUserWidget::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
	OnHide();
}
