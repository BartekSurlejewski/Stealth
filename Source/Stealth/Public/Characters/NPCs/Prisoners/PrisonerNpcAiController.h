#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/NpcAiController.h"
#include "PrisonerNpcAiController.generated.h"

UCLASS()
class STEALTH_API APrisonerNpcAiController : public ANpcAiController
{
	GENERATED_BODY()

public:
	APrisonerNpcAiController();

protected:
	virtual void BeginPlay() override;
	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override;

public:
	virtual void Tick(float DeltaTime) override;
};
