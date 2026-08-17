#include "Characters/NPCs/Guards/GuardNpcContextComponent.h"

#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/Guards/NpcPatrolComponent.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Stealth/Stealth.h"


class UStateTreeComponent;

void UGuardNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();

	PatrolComponent = NpcAiController->GetPawn()->FindComponentByClass<UNpcPatrolComponent>();
}

AActor* UGuardNpcContextComponent::GetCurrentPatrolPoint() const
{
	if (!PatrolComponent)
	{
		return nullptr;
	}

	return PatrolComponent->GetCurrentTarget();
}

void UGuardNpcContextComponent::IncrementPatrolIndex() const
{
	if (!PatrolComponent)
	{
		return;
	}

	PatrolComponent->IncrementTargetIndex();
}

void UGuardNpcContextComponent::SetBehaviourState(const EGuardBehaviourState& NewBehaviourState)
{
	UE_LOG(LogStealth, Log, TEXT("[%s] Changing state to %hhd"), *GetOwner()->GetName(), NewBehaviourState);

	if (BehaviourState != NewBehaviourState)
	{
		BehaviourState = NewBehaviourState;
		OnBehaviourStateChanged.Broadcast(BehaviourState);
	}
}

bool UGuardNpcContextComponent::IsOnWalkingPatrol() const
{
	//TODO: this won't work at the scene start, since state tree seems to call it before BeginPlay. Fix somehow
	if (!PatrolComponent)
	{
		return false;
	}

	return PatrolComponent->IsOnWalkingPatrol();
}


void UGuardNpcContextComponent::LookAtPlayer()
{
	if (!NpcAiController)
	{
		return;
	}

	FVector DirectionToPlayer = UCharactersRegistrySubsystem::Get(this)->GetPlayerCharacter()->GetActorLocation() - NpcAiController->GetPawn()->GetActorLocation();
	FRotator LookAtRotation = DirectionToPlayer.Rotation();
	// Only rotate on Yaw axis, keep current Pitch and Roll
	LookAtRotation.Pitch = 0.f;
	LookAtRotation.Roll = 0.f;

	NpcAiController->GetPawn()->SetActorRotation(LookAtRotation);
}

void UGuardNpcContextComponent::OnAlarmChanged(const int32 NewLevel, const FVector& SourceLocation)
{
	GlobalAlarmLevel = NewLevel;
	if (NewLevel >= 2) {}
}
