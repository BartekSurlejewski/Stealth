#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "StealthPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnIsInRestrictedAreaChanged, bool, IsInRestricterArea);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPerformedIllegalAction);

UCLASS()
class STEALTH_API AStealthPlayerState : public APlayerState
{
	GENERATED_BODY()

	/*Events*/
public:
	//TODO: Create some kind of legality subsystem?
	UPROPERTY(BlueprintAssignable)
	FOnIsInRestrictedAreaChanged OnIsInRestrictedAreaChanged;
	UPROPERTY(BlueprintAssignable)
	FOnPerformedIllegalAction OnPerformedIllegalAction;

	/*Properties*/
private:
	UPROPERTY(Category = PlayerState, BlueprintGetter=GetIsInRestrictedArea)
	bool bIsInRestrictedArea = false;

	/*Methods*/
public:
	UFUNCTION(BlueprintGetter)
	bool GetIsInRestrictedArea() const
	{
		return bIsInRestrictedArea;
	}

	UFUNCTION()
	void SetIsInRestrictedArea(bool newIsInRestrictedArea)
	{
		bIsInRestrictedArea = newIsInRestrictedArea;
		OnIsInRestrictedAreaChanged.Broadcast(bIsInRestrictedArea);
	}
};
