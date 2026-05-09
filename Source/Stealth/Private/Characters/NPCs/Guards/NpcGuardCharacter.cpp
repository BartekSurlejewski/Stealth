#include "Characters/NPCs/Guards/NpcGuardCharacter.h"

#include "Characters/NPCs/Guards/NpcPatrolComponent.h"


ANpcGuardCharacter::ANpcGuardCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	PatrolComponent = CreateDefaultSubobject<UNpcPatrolComponent>(TEXT("Patrol Component"));
}
