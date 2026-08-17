#pragma once

#include "CoreMinimal.h"
#include "StealthBaseWidget.h"
#include "StealthBaseHudWidget.generated.h"


UCLASS(Abstract, Blueprintable)
class STEALTH_API UStealthBaseHudWidget : public UStealthBaseWidget
{
	GENERATED_BODY()

protected:
	virtual EUILayer GetLayer() const override { return EUILayer::HUD; }
};
