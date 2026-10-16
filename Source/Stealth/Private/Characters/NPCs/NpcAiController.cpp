#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/AI/Perception/NpcSensorComponent.h"
#include "Characters/NPCs/AI/Perception/NpcAwarenessComponent.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Components/StateTreeAIComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig.h"

ANpcAiController::ANpcAiController()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("Perception Component");
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>("State Tree AI Component");
	NpcInventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory Component");
	SensorComponent = CreateDefaultSubobject<UNpcSensorComponent>("Sensor Component");
	AwarenessComponent = CreateDefaultSubobject<UNpcAwarenessComponent>("Awareness Component");
	FocusComponent = CreateDefaultSubobject<UNpcFocusComponent>("Focus Component");
}

void ANpcAiController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!ContextComponent)
	{
		ContextComponent = FindComponentByClass<UNpcContextComponent>();
	}

	if (PerceptionComponent)
	{
		PerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ANpcAiController::OnTargetPerceptionUpdated);
		PerceptionComponent->OnTargetPerceptionForgotten.AddUniqueDynamic(this, &ANpcAiController::OnTargetPerceptionForgotten);
	}
}

void ANpcAiController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!ContextComponent)
	{
		ContextComponent = FindComponentByClass<UNpcContextComponent>();
	}
}

void ANpcAiController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (SensorComponent)
	{
		SensorComponent->HandleStimulus(Actor, Stimulus);
	}
}

void ANpcAiController::OnTargetPerceptionForgotten(AActor* Actor)
{
	if (SensorComponent)
	{
		SensorComponent->HandleForgotten(Actor);
	}
}

const UNpcProfile* ANpcAiController::GetNpcProfile() const
{
	if (ContextComponent)
	{
		return ContextComponent->GetNpcProfile();
	}
	return nullptr;
}
