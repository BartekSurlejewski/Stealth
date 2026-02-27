// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/NpcCharacter.h"
#include "NpcGuardCharacter.generated.h"

class UNpcPatrolComponent;
/**
 * 
 */
UCLASS()
class STEALTH_API ANpcGuardCharacter : public ANpcCharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UNpcPatrolComponent> PatrolComponent;

public:
	ANpcGuardCharacter();
};
