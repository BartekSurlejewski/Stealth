#include "Characters/NPCs/Guards/GuardNpcAiController.h"

#include "Characters/NPCs/Guards/GuardNpcContextComponent.h"
#include "Exposure/PlayerExposureSubsystem.h"
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

	if (!GuardContext) return;

	// Sight
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		FAIStimulus ModifiedStimulus = Stimulus;

		float ExposureMultiplier = 1.0f;
		if (Stimulus.WasSuccessfullySensed() && LightExposureSubsystem)
		{
			ExposureMultiplier = LightExposureSubsystem->GetCurrentTotalExposure();
		}

		GuardContext->OnSightStimulus(TargetActor, ModifiedStimulus, ExposureMultiplier);
		return;
	}

	// Hearing
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		GuardContext->OnHearingStimulus(TargetActor, Stimulus);
		return;
	}
}
