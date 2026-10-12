#include "Characters/NPCs/NpcAiController.h"

#include "Characters/NPCs/NpcCharacter.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/AI/Perception/NpcSensorComponent.h"
#include "Characters/NPCs/AI/Perception/NpcAwarenessComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Prediction.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Touch.h"
#include "Perception/AISense_Damage.h"

ANpcAiController::ANpcAiController()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("Perception Component");
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>("State Tree AI Component");
	NpcInventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory Component");
	SensorComponent = CreateDefaultSubobject<UNpcSensorComponent>("Sensor Component");
	AwarenessComponent = CreateDefaultSubobject<UNpcAwarenessComponent>("Awareness Component");
	FocusComponent = CreateDefaultSubobject<UNpcFocusComponent>("Focus Component");
}

void ANpcAiController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CachedNpcContextComponent = FindComponentByClass<UNpcContextComponent>();

	if (const ANpcCharacter* NpcCharacter = Cast<ANpcCharacter>(InPawn))
	{
		NpcGUID = NpcCharacter->GetNpcGUID();
	}

	if (PerceptionComponent)
	{
		PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ANpcAiController::OnTargetPerceptionUpdated);
		PerceptionComponent->OnTargetPerceptionForgotten.AddDynamic(this, &ANpcAiController::OnTargetPerceptionForgotten);
	}
}

void ANpcAiController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PerceptionComponent)
	{
		PerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this);
		PerceptionComponent->OnTargetPerceptionForgotten.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ANpcAiController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
	{
		if (SensorComponent)
		{
			SensorComponent->OnSightStimulus(Actor, Stimulus);
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (SensorComponent)
		{
			SensorComponent->OnHearingStimulus(Actor, Stimulus);
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Prediction>())
	{
		if (SensorComponent)
		{
			SensorComponent->OnPredictionStimulus(Actor, Stimulus);
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Touch>())
	{
		if (SensorComponent)
		{
			SensorComponent->OnTouchStimulus(Actor, Stimulus);
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Damage>())
	{
		if (SensorComponent)
		{
			SensorComponent->OnDamageStimulus(Actor, Stimulus);
		}
	}
}

void ANpcAiController::OnTargetPerceptionForgotten(AActor* Actor) {}

UNpcContextComponent* ANpcAiController::GetContextComponent() const
{
	if (!CachedNpcContextComponent.IsValid())
	{
		CachedNpcContextComponent = FindComponentByClass<UNpcContextComponent>();
	}

	return CachedNpcContextComponent.Get();
}
