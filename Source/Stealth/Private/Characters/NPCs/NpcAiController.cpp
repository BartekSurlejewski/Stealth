#include "Characters/NPCs/NpcAiController.h"

#include "Characters/NPCs/NpcCharacter.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/AI/Suspicion/NpcPerceptionComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Prediction.h"
#include "Perception/AISense_Sight.h"


ANpcAiController::ANpcAiController()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("Perception Component");
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>("State Tree AI Component");
	NpcInventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory Component");
	NpcPerceptionComponent = CreateDefaultSubobject<UNpcPerceptionComponent>("Suspicion Component");
	FocusComponent = CreateDefaultSubobject<UNpcFocusComponent>("Focus Component");
}

void ANpcAiController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

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
		if (NpcPerceptionComponent)
		{
			NpcPerceptionComponent->OnSightStimulus(Actor, Stimulus);
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Hearing>())
	{
		if (NpcPerceptionComponent)
		{
			NpcPerceptionComponent->OnHearingStimulus(Actor, Stimulus);
		}
	}
	else if (Stimulus.Type == UAISense::GetSenseID<UAISense_Prediction>())
	{
		if (NpcPerceptionComponent)
		{
			NpcPerceptionComponent->OnPredictionStimulus(Actor, Stimulus);
		}
	}
}

void ANpcAiController::OnTargetPerceptionForgotten(AActor* Actor) {}
