#include "Characters/NPCs/Guards/GuardNpcContextComponent.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/Guards/NpcPatrolComponent.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Stealth/Stealth.h"

void UGuardNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();
	GetPatrolComponent();
}

UNpcPatrolComponent* UGuardNpcContextComponent::GetPatrolComponent() const
{
	if (!PatrolComponent)
	{
		if (NpcAiController && NpcAiController->GetPawn())
		{
			PatrolComponent = NpcAiController->GetPawn()->FindComponentByClass<UNpcPatrolComponent>();
		}
		else if (const APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			PatrolComponent = Pawn->FindComponentByClass<UNpcPatrolComponent>();
		}
		else if (GetOwner())
		{
			PatrolComponent = GetOwner()->FindComponentByClass<UNpcPatrolComponent>();
		}
	}
	return PatrolComponent.Get();
}

AActor* UGuardNpcContextComponent::GetCurrentPatrolPoint() const
{
	if (UNpcPatrolComponent* PatrolComp = GetPatrolComponent())
	{
		return PatrolComp->GetCurrentTarget();
	}
	return nullptr;
}

void UGuardNpcContextComponent::IncrementPatrolIndex() const
{
	if (UNpcPatrolComponent* PatrolComp = GetPatrolComponent())
	{
		PatrolComp->IncrementTargetIndex();
	}
}

bool UGuardNpcContextComponent::IsOnWalkingPatrol() const
{
	if (UNpcPatrolComponent* PatrolComp = GetPatrolComponent())
	{
		return PatrolComp->IsOnWalkingPatrol();
	}
	return false;
}

void UGuardNpcContextComponent::LookAtPlayer()
{
	if (!NpcAiController || !NpcAiController->GetPawn())
	{
		return;
	}

	const AStealthPlayerCharacter* Player = GetPlayerCharacter();
	if (!Player)
	{
		return;
	}

	FVector DirectionToPlayer = Player->GetActorLocation() - NpcAiController->GetPawn()->GetActorLocation();
	FRotator LookAtRotation = DirectionToPlayer.Rotation();
	LookAtRotation.Pitch = 0.0f;
	LookAtRotation.Roll = 0.0f;

	NpcAiController->GetPawn()->SetActorRotation(LookAtRotation);
}

void UGuardNpcContextComponent::OnAlarmChanged(const int32 NewLevel, const FVector& SourceLocation)
{
	GlobalAlarmLevel = NewLevel;
	if (NewLevel >= 2)
	{
		SetAlertLevel(ENpcAlertLevel::Hostile);
	}
}
