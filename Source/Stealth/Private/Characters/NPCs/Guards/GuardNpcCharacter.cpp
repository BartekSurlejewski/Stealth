#include "Characters/NPCs/Guards/GuardNpcCharacter.h"

#include "Characters/NPCs/Guards/NpcPatrolComponent.h"


AGuardNpcCharacter::AGuardNpcCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	PatrolComponent = CreateDefaultSubobject<UNpcPatrolComponent>(TEXT("Patrol Component"));
}
