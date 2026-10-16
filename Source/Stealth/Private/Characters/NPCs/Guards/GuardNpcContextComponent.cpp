#include "Characters/NPCs/Guards/GuardNpcContextComponent.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/AI/Perception/NpcAwarenessComponent.h"
#include "Characters/NPCs/AI/Patrol/PatrolRoute.h"
#include "Characters/NPCs/AI/Patrol/PatrolSubsystem.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Stealth/Stealth.h"

void UGuardNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();
}

APatrolRoute* UGuardNpcContextComponent::GetActivePatrolRoute() const
{
	return ActivePatrolRoute.Get();
}

void UGuardNpcContextComponent::SetActivePatrolRoute(APatrolRoute* NewRoute)
{
	ActivePatrolRoute = NewRoute;
	CurrentPatrolIndex = 0;
	bMovingForward = true;
}

APatrolRoute* UGuardNpcContextComponent::AssignPatrolRouteForLocation(const FGameplayTag& LocationTag)
{
	if (!LocationTag.IsValid())
	{
		ActivePatrolRoute = nullptr;
		return nullptr;
	}

	if (UPatrolSubsystem* Subsystem = UPatrolSubsystem::Get(this))
	{
		FVector RequesterPos = FVector::ZeroVector;
		if (const APawn* Pawn = GetNpcPawn())
		{
			RequesterPos = Pawn->GetActorLocation();
		}

		APatrolRoute* FoundRoute = Subsystem->GetPatrolRouteByLocationTag(LocationTag, RequesterPos);
		if (FoundRoute)
		{
			ActivePatrolRoute = FoundRoute;
			ResumePatrol();
			return FoundRoute;
		}
	}

	ActivePatrolRoute = nullptr;
	return nullptr;
}

AActor* UGuardNpcContextComponent::GetCurrentPatrolPoint() const
{
	if (const APatrolRoute* Route = GetActivePatrolRoute())
	{
		return Route->GetWaypoint(CurrentPatrolIndex);
	}
	return nullptr;
}

void UGuardNpcContextComponent::IncrementPatrolIndex()
{
	if (const APatrolRoute* Route = GetActivePatrolRoute())
	{
		CurrentPatrolIndex = Route->GetNextWaypointIndex(CurrentPatrolIndex, bMovingForward);
	}
}

void UGuardNpcContextComponent::ResumePatrol()
{
	if (const APatrolRoute* Route = GetActivePatrolRoute())
	{
		FVector RequesterPos = FVector::ZeroVector;
		if (const APawn* Pawn = GetNpcPawn())
		{
			RequesterPos = Pawn->GetActorLocation();
		}

		if (!RequesterPos.IsZero())
		{
			CurrentPatrolIndex = Route->GetClosestWaypointIndex(RequesterPos);
		}
	}
}

void UGuardNpcContextComponent::ResetPatrol()
{
	CurrentPatrolIndex = 0;
	bMovingForward = true;
}

bool UGuardNpcContextComponent::IsOnWalkingPatrol() const
{
	const APatrolRoute* Route = GetActivePatrolRoute();
	return (Route != nullptr && Route->HasValidWaypoints());
}

AActor* UGuardNpcContextComponent::GetStandingGuardSpot() const
{
	if (const APatrolRoute* Route = GetActivePatrolRoute())
	{
		return Route->GetStandingGuardSpot();
	}
	return nullptr;
}

bool UGuardNpcContextComponent::HasStandingGuardSpot() const
{
	return GetStandingGuardSpot() != nullptr;
}

void UGuardNpcContextComponent::LookAtPlayer()
{
	const APawn* Pawn = GetNpcPawn();
	if (!Pawn)
	{
		return;
	}

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		return;
	}

	FVector DirectionToPlayer = PlayerPawn->GetActorLocation() - Pawn->GetActorLocation();
	FRotator LookAtRotation = DirectionToPlayer.Rotation();
	LookAtRotation.Pitch = 0.0f;
	LookAtRotation.Roll = 0.0f;

	const_cast<APawn*>(Pawn)->SetActorRotation(LookAtRotation);
}

void UGuardNpcContextComponent::OnAlarmChanged(const int32 NewLevel, const FVector& SourceLocation)
{
	GlobalAlarmLevel = NewLevel;
	if (NewLevel >= 2)
	{
		if (const ANpcAiController* Controller = GetNpcController())
		{
			if (UNpcAwarenessComponent* Awareness = Controller->GetAwarenessComponent())
			{
				Awareness->AddSuspicion(100.0f);
			}
		}
		SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_CrimeReported);
	}
}
