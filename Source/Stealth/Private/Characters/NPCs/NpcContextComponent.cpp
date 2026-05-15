#include "Characters/NPCs/NpcContextComponent.h"

#include "Characters/NPCs/NpcAiController.h"


UNpcContextComponent::UNpcContextComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz
}


void UNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();

	StateTreeComponent = GetOwner()->FindComponentByClass<UStateTreeAIComponent>();
	NpcAiController = Cast<ANpcAiController>(GetOwner());
}
