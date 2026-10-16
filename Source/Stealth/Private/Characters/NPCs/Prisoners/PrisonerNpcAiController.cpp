#include "Characters/NPCs/Prisoners/PrisonerNpcAiController.h"

#include "Characters/NPCs/Prisoners/PrisonerNpcContextComponent.h"
#include "Perception/AIPerceptionTypes.h"


APrisonerNpcAiController::APrisonerNpcAiController()
{
	PrisonerContext = CreateDefaultSubobject<UPrisonerNpcContextComponent>(TEXT("Prisoner Context Component"));
	ContextComponent = PrisonerContext;
}

void APrisonerNpcAiController::BeginPlay()
{
	Super::BeginPlay();
}

void APrisonerNpcAiController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	Super::OnTargetPerceptionUpdated(Actor, Stimulus);
}
