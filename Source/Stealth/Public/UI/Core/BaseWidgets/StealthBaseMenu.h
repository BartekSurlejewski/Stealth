#pragma once

#include "CoreMinimal.h"
#include "StealthBaseWidget.h"
#include "StealthBaseMenu.generated.h"


UCLASS(Abstract, Blueprintable)
class STEALTH_API UStealthBaseMenu : public UStealthBaseWidget
{
	GENERATED_BODY()

public:
	virtual EUILayer GetLayer() const override { return EUILayer::Menu; }
};
