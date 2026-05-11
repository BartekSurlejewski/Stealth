#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/NpcAiController.h"
#include "GuardNpcAiController.generated.h"

class UGuardNpcContextComponent;

UCLASS()
class STEALTH_API AGuardNpcAiController : public ANpcAiController
{
	GENERATED_BODY()

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnTargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus) override;

	UPROPERTY()
	TObjectPtr<UGuardNpcContextComponent> GuardContext;
};
