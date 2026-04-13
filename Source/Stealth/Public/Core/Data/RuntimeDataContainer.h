#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "RuntimeDataContainer.generated.h"

UCLASS(Abstract, Blueprintable)
class STEALTH_API ARuntimeDataContainer : public AInfo
{
	GENERATED_BODY()

public:
	ARuntimeDataContainer();
};
