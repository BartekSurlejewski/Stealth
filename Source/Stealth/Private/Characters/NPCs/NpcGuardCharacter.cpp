// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NPCs/NpcGuardCharacter.h"

#include "Characters/NPCs/NpcPatrolComponent.h"

ANpcGuardCharacter::ANpcGuardCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	PatrolComponent = CreateDefaultSubobject<UNpcPatrolComponent>(TEXT("Patrol Component"));
}
