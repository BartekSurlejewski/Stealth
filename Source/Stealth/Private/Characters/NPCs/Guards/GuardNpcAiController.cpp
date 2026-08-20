#include "Characters/NPCs/Guards/GuardNpcAiController.h"

#include "Characters/NPCs/Guards/GuardNpcContextComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"


AGuardNpcAiController::AGuardNpcAiController()
{
	GuardContext = CreateDefaultSubobject<UGuardNpcContextComponent>(TEXT("Guard Context Component"));
}

void AGuardNpcAiController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AGuardNpcAiController::OnTargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	Super::OnTargetPerceptionUpdated(TargetActor, Stimulus);
}
