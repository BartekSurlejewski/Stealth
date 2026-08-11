#include "Characters/NPCs/NpcAiController.h"

#include "Characters/NPCs/NpcCharacter.h"
#include "Components/StateTreeAIComponent.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/InventoryManagerSubsystem.h"
#include "Perception/AIPerceptionComponent.h"


ANpcAiController::ANpcAiController()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("Perception Component");
	StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>("State Tree AI Component");
	NpcInventoryComponent = CreateDefaultSubobject<UInventoryComponent>("Inventory Component");
}

void ANpcAiController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (const ANpcCharacter* NpcCharacter = Cast<ANpcCharacter>(InPawn))
	{
		if (UInventoryManagerSubsystem* InventoryManager = UInventoryManagerSubsystem::Get(this))
		{
			InventoryManager->RegisterNpcInventory(NpcGUID, NpcInventoryComponent);
		}

		NpcGUID = NpcCharacter->GetNpcGUID();
	}

	if (PerceptionComponent)
	{
		PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ANpcAiController::OnTargetPerceptionUpdated);
	}
}

void ANpcAiController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UInventoryManagerSubsystem* InventoryManager = UInventoryManagerSubsystem::Get(this))
	{
		InventoryManager->UnregisterNpcInventory(NpcGUID, NpcInventoryComponent);
	}

	if (PerceptionComponent)
	{
		PerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ANpcAiController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) {}
