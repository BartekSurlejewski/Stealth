#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/NpcAiController.h"
#include "GuardNpcAiController.generated.h"

class UGuardNpcContextComponent;

UCLASS()
class STEALTH_API AGuardNpcAiController : public ANpcAiController
{
	GENERATED_BODY()

public:
	AGuardNpcAiController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnTargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGuardNpcContextComponent> GuardContext;
};
