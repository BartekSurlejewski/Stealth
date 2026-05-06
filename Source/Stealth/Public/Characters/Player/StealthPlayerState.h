#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "StealthPlayerState.generated.h"

UCLASS()
class STEALTH_API AStealthPlayerState : public APlayerState
{
	GENERATED_BODY()

private:
	UPROPERTY(Category = PlayerState, BlueprintGetter=GetIsInRestrictedArea)
	bool bIsInRestrictedArea = false;

public:
	UFUNCTION(BlueprintGetter)
	bool GetIsInRestrictedArea() const
	{
		return bIsInRestrictedArea;
	}
};
