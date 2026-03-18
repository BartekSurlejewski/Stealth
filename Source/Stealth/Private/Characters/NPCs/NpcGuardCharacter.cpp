#include "Characters/NPCs/NpcGuardCharacter.h"

#include "Characters/NPCs/NpcPatrolComponent.h"
#include "Perception/AIPerceptionComponent.h"

ANpcGuardCharacter::ANpcGuardCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	PatrolComponent = CreateDefaultSubobject<UNpcPatrolComponent>(TEXT("Patrol Component"));
	AiPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AI Perception"));
}
