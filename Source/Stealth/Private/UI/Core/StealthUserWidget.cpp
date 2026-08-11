#include "UI/Core/StealthUserWidget.h"

void UStealthUserWidget::Show()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UStealthUserWidget::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}
