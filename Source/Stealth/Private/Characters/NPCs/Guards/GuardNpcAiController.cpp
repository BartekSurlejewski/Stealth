#include "Characters/NPCs/Guards/GuardNpcAiController.h"

#include "Characters/NPCs/Guards/GuardNpcContextComponent.h"
#include "Characters/NPCs/Guards/GuardNpcProfile.h"
#include "Exposure/PlayerExposureSubsystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"


void AGuardNpcAiController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	GuardContext = InPawn->FindComponentByClass<UGuardNpcContextComponent>();

	if (GuardContext && GuardContext->Profile)
	{
		UGuardNpcProfile* Profile = GuardContext->Profile;

		auto* Sight = NewObject<UAISenseConfig_Sight>(this);
		Sight->SightRadius = Profile->SightRadius;
		Sight->LoseSightRadius = Profile->LoseSightRadius;
		Sight->PeripheralVisionAngleDegrees = Profile->PeripheralVisionAngleDegrees;
		Sight->SetMaxAge(5.f);
		Sight->DetectionByAffiliation.bDetectEnemies = true;
		Sight->DetectionByAffiliation.bDetectNeutrals = true;
		PerceptionComponent->ConfigureSense(*Sight);

		auto* Hearing = NewObject<UAISenseConfig_Hearing>(this);
		Hearing->HearingRange = Profile->HearingRadius;
		Hearing->SetMaxAge(3.f);
		Hearing->DetectionByAffiliation.bDetectEnemies = true;
		Hearing->DetectionByAffiliation.bDetectNeutrals = true;
		PerceptionComponent->ConfigureSense(*Hearing);

		PerceptionComponent->SetDominantSense(Sight->GetSenseImplementation());
		PerceptionComponent->RequestStimuliListenerUpdate();
	}
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
