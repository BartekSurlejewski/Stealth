// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Controllers/NpcAiController.h"

#include "Characters/Player/StealthCharacter.h"
#include "Components/StateTreeAIComponent.h"
#include "Lighting/LightExposureSubsystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

ANpcAiController::ANpcAiController()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("Perception Component");
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>("State Tree AI Component");
}

void ANpcAiController::BeginPlay()
{
	Super::BeginPlay();

	if (!PerceptionComponent)
	{
		return;
	}

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ANpcAiController::OnTargetPerceptionUpdated);
}

void ANpcAiController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	PerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this);
}

void ANpcAiController::OnTargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus)
{
	if (!Cast<AStealthCharacter>(TargetActor))
	{
		return;
	}

	if (!Stimulus.WasSuccessfullySensed() || Stimulus.Type != UAISense::GetSenseID<UAISense_Sight>())
	{
		return;
	}

	if (!LightExposureSubsystem)
	{
		LightExposureSubsystem = GetWorld()->GetSubsystem<ULightExposureSubsystem>();
		if (!LightExposureSubsystem)
		{
			return;
		}
	}

	float ActorVisibilityStrength = Stimulus.Strength * LightExposureSubsystem->GetPlayerLightExposure();
	if (FMath::IsNearlyEqual(ActorVisibilityStrength, 0.f, 0.01f))
	{
		CharacterState.bSeesPlayer = false;
	}
	if (CharacterState.bIsAwareOfPlayer)
	{
		CharacterState.bSeesPlayer = ActorVisibilityStrength > 0.5f;
	}
	else
	{
		CharacterState.bSeesPlayer = ActorVisibilityStrength > 0.25f;
	}

	FStateTreeEvent Event;
	Event.Tag = ChangedStateEventTag;
	StateTreeAIComponent->SendStateTreeEvent(Event);
}
