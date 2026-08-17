#pragma once

#include "CoreMinimal.h"
#include "StealthBaseWidget.h"
#include "StealthBaseWorldSpaceWidget.generated.h"

UCLASS()
class STEALTH_API UStealthBaseWorldSpaceWidget : public UStealthBaseWidget
{
	GENERATED_BODY()

public:
	virtual EUILayer GetLayer() const override { return EUILayer::WorldSpace; }
};
