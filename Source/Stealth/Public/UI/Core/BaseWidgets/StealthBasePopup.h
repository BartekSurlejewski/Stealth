#pragma once

#include "CoreMinimal.h"
#include "StealthBaseWidget.h"
#include "StealthBasePopup.generated.h"


UCLASS(Abstract, Blueprintable)
class STEALTH_API UStealthBasePopup : public UStealthBaseWidget
{
	GENERATED_BODY()

protected:
	virtual EUILayer GetLayer() const override { return EUILayer::Popup; }
};
