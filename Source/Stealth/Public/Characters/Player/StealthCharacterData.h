#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthCharacterData.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthCharacterData : public UActorComponent
{
	GENERATED_BODY()

public:
	UStealthCharacterData();

	UPROPERTY()
	bool bIsCrouched = false;
};
