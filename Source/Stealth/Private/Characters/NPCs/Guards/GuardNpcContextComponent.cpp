#include "Characters/NPCs/Guards/GuardNpcContextComponent.h"

#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/Guards/NpcPatrolComponent.h"
#include "Characters/Player/StealthPlayerState.h"
// #include "Components/StateTreeComponent.h"
// #include "Perception/AIPerceptionTypes.h"
#include "Stealth/Stealth.h"


class UStateTreeComponent;

void UGuardNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();

	PatrolComponent = NpcAiController->GetPawn()->FindComponentByClass<UNpcPatrolComponent>();

	if (GetPlayerState())
	{
		GetPlayerState()->OnPerformedIllegalAction.AddDynamic(this, &UGuardNpcContextComponent::OnPlayerPerformedIllegalAction);
		GetPlayerState()->OnIsInRestrictedAreaChanged.AddDynamic(this, &UGuardNpcContextComponent::OnIsPlayerInRestrictedAreaChanged);
	}
}

void UGuardNpcContextComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (GetPlayerState())
	{
		GetPlayerState()->OnPerformedIllegalAction.RemoveAll(this);
		GetPlayerState()->OnIsInRestrictedAreaChanged.RemoveAll(this);
	}
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

	FVector DirectionToPlayer = PlayerPawn->GetActorLocation() - NpcAiController->GetPawn()->GetActorLocation();
	FRotator LookAtRotation = DirectionToPlayer.Rotation();
	// Only rotate on Yaw axis, keep current Pitch and Roll
	LookAtRotation.Pitch = 0.f;
	LookAtRotation.Roll = 0.f;

	NpcAiController->GetPawn()->SetActorRotation(LookAtRotation);
}

void UGuardNpcContextComponent::OnPlayerPerformedIllegalAction()
{
	if (!bPlayerInSight)
	{
		// Guard doesn't see player, do nothing
		return;
	}

	SendStateTreeEvent(SuspiciousActivityTag);
}

void UGuardNpcContextComponent::OnIsPlayerInRestrictedAreaChanged(bool bIsInRestrictedArea)
{
	if (!bIsInRestrictedArea || !bPlayerInSight)
	{
		// Guard doesn't see player, do nothing
		return;
	}

	SendStateTreeEvent(SuspiciousActivityTag);
}

void UGuardNpcContextComponent::OnAlarmChanged(const int32 NewLevel, const FVector& SourceLocation)
{
	GlobalAlarmLevel = NewLevel;
	if (NewLevel >= 2) {}
}
