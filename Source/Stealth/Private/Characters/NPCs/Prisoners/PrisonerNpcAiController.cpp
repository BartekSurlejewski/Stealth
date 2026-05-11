#include "Characters/NPCs/Prisoners/PrisonerNpcAiController.h"

#include "Perception/AIPerceptionTypes.h"


// Sets default values
APrisonerNpcAiController::APrisonerNpcAiController()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APrisonerNpcAiController::BeginPlay()
{
	Super::BeginPlay();
}

void APrisonerNpcAiController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	Super::OnTargetPerceptionUpdated(Actor, Stimulus);
}

void APrisonerNpcAiController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
