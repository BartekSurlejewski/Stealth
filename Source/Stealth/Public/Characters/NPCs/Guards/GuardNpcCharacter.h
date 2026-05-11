#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/NpcCharacter.h"
#include "GuardNpcCharacter.generated.h"

class UAIPerceptionComponent;
class UNpcPatrolComponent;

UCLASS()
class STEALTH_API AGuardNpcCharacter : public ANpcCharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UNpcPatrolComponent> PatrolComponent;

public:
	AGuardNpcCharacter();
};
