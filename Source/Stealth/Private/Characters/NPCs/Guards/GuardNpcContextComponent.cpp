#include "Characters/NPCs/Guards/GuardNpcContextComponent.h"

#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/Guards/GuardNpcProfile.h"
#include "Characters/NPCs/Guards/NpcPatrolComponent.h"
#include "Characters/Player/StealthPlayerState.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionTypes.h"
#include "Stealth/Stealth.h"


class UStateTreeComponent;

void UGuardNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();

	PatrolComponent = NpcAiController->GetPawn()->FindComponentByClass<UNpcPatrolComponent>();

	if (GetPlayerState())
	{
		GetPlayerState()->OnPerformedIllegalAction.AddDynamic(this, &UGuardNpcContextComponent::OnPlayerPerformedIllegalAction);
		GetPlayerState()->OnIsInRestrictedAreaChanged.AddDynamic(this, &UGuardNpcContextComponent::OnIsInRestrictedAreaChanged);
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

//TODO: Add changing to suspicious state when guard sees player doing something illegal, 
// then to Alert state when player does it again in a given time period
void UGuardNpcContextComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bPlayerInDirectSight || bPlayerInPeripheralSight)
	{
		LastKnownPlayerPos = GetPlayerPawn()->GetActorLocation();
	}

	if (BehaviourState == EGuardBehaviourState::Patrol && bPlayerInDirectSight && GetPlayerState()->GetIsInRestrictedArea())
	{
		// Player is seen and is in restricted area!
		SendGuardEvent(GotSuspiciousTag);
		UE_LOG(LogStealth, Log, TEXT("[%s] Player is in restricted area, seems sus!"), *GetOwner()->GetName());
	}
	else if (BehaviourState == EGuardBehaviourState::Suspicious)
	{
		if (!bPlayerInDirectSight || !GetPlayerState()->GetIsInRestrictedArea())
		{
			SuspicionDownTimer -= DeltaTime;
			UE_LOG(LogStealth, Log, TEXT("[%s] Suspicion timer: %f"), *GetOwner()->GetName(), SuspicionDownTimer);
			if (SuspicionDownTimer <= 0.f)
			{
				SendGuardEvent(SearchExpiredTag);
				UE_LOG(LogStealth, Log, TEXT("[%s] Suspicion expired"), *GetOwner()->GetName());
			}
		}
		else if (bPlayerInDirectSight && GetPlayerState()->GetIsInRestrictedArea())
		{
			SuspicionToAlertTimer -= DeltaTime;
			UE_LOG(LogStealth, Log, TEXT("[%s] Suspicion to alert timer: %f"), *GetOwner()->GetName(), SuspicionToAlertTimer);
			if (SuspicionToAlertTimer <= 0.f)
			{
				SendGuardEvent(AlertedTag);
				UE_LOG(LogStealth, Log, TEXT("[%s] Changing from suspicious to alerted"), *GetOwner()->GetName());
			}
		}
	}
	else if (BehaviourState == EGuardBehaviourState::Alerted)
	{
		if (bPlayerInDirectSight)
		{
			AlertedWithoutSeeingTimer = Profile->AlertWithoutSeeingDuration;
		}
		else
		{
			AlertedWithoutSeeingTimer -= DeltaTime;
			if (AlertedWithoutSeeingTimer <= 0.f)
			{
				SendGuardEvent(PlayerLostTag);
			}
		}
	}
	else if (BehaviourState == EGuardBehaviourState::Search && SearchTimer > 0.f)
	{
		SearchTimer -= DeltaTime;
		if (SearchTimer <= 0.f)
		{
			SendGuardEvent(SearchExpiredTag);
		}
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

	//TODO: Add a class for each state logic to avoid constant if-checks
	if (BehaviourState == EGuardBehaviourState::Suspicious)
	{
		SuspicionDownTimer = Profile ? Profile->SuspicionDuration : 30.f;
		SuspicionToAlertTimer = Profile ? Profile->SuspicionDuration : 30.f;
	}
	else if (BehaviourState == EGuardBehaviourState::Search)
	{
		SearchTimer = Profile ? Profile->SearchDuration : 20.f;
	}
}

void UGuardNpcContextComponent::ForceAlert(FVector AtLocation)
{
	LastKnownPlayerPos = AtLocation;
	SendGuardEvent(GotSuspiciousTag);
}

void UGuardNpcContextComponent::BeginSearch()
{
	SearchTimer = Profile ? Profile->SearchDuration : 20.f;
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

void UGuardNpcContextComponent::OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus, float ExposureMultiplier)
{
	if (Actor != GetPlayerPawn())
	{
		return;
	}

	const float EffectiveStrength = Stimulus.Strength * ExposureMultiplier;
	//TODO: add defining effectiveStrength threshold without hardcoding it
	if (Stimulus.WasSuccessfullySensed() && EffectiveStrength >= 0.05f)
	{
		// LastKnownPlayerPos = Stimulus.StimulusLocation;
		bPlayerInDirectSight = EffectiveStrength >= 0.8f;
		bPlayerInPeripheralSight = EffectiveStrength < 0.8f;
	}
	else
	{
		bPlayerInDirectSight = false;
		bPlayerInPeripheralSight = false;
	}

	UE_LOG(LogStealth, Log, TEXT("[%s] Player in direct sight: %s"), *GetOwner()->GetName(), bPlayerInDirectSight ? TEXT("true") : TEXT("false"));
	UE_LOG(LogStealth, Log, TEXT("[%s] Player in peripheral sight: %s"), *GetOwner()->GetName(), bPlayerInDirectSight ? TEXT("true") : TEXT("false"));
	OnPlayerInSightChanged.Broadcast(bPlayerInDirectSight, bPlayerInPeripheralSight);
}

void UGuardNpcContextComponent::OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed() || Actor != GetPlayerPawn())
	{
		return;
	}

	LastHeardSoundLocation = Stimulus.StimulusLocation;

	if (BehaviourState <= EGuardBehaviourState::Suspicious)
	{
		LastKnownPlayerPos = Stimulus.StimulusLocation;
	}
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
	if (!bPlayerInDirectSight)
	{
		// Guard doesn't see player, do nothing
		return;
	}

	if (BehaviourState == EGuardBehaviourState::Patrol)
	{
		SendGuardEvent(GotSuspiciousTag);
	}
	else if (BehaviourState == EGuardBehaviourState::Suspicious)
	{
		SendGuardEvent(AlertedTag);
	}
}

void UGuardNpcContextComponent::OnIsInRestrictedAreaChanged(bool bIsInRestrictedArea)
{
	if (bIsInRestrictedArea && bPlayerInDirectSight && BehaviourState == EGuardBehaviourState::Suspicious)
	{
		SuspicionDownTimer = Profile ? Profile->SuspicionDuration : 30.f;
		SuspicionToAlertTimer = Profile ? Profile->SuspicionDuration : 30.f;
	}
}

void UGuardNpcContextComponent::SendGuardEvent(const FGameplayTag Tag) const
{
	if (!StateTreeComponent)
	{
		return;
	}
	StateTreeComponent->SendStateTreeEvent(FStateTreeEvent(Tag));
}

void UGuardNpcContextComponent::OnAlarmChanged(const int32 NewLevel, const FVector& SourceLocation)
{
	GlobalAlarmLevel = NewLevel;
	if (NewLevel >= 2)
	{
		if (BehaviourState <= EGuardBehaviourState::Suspicious)
		{
			LastKnownPlayerPos = SourceLocation;
		}

		SendGuardEvent(GlobalAlarmTag);
	}
}

APawn* UGuardNpcContextComponent::GetPlayerPawn()
{
	if (PlayerPawn == nullptr)
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	return PlayerPawn.Get();
}

AStealthPlayerState* UGuardNpcContextComponent::GetPlayerState()
{
	if (PlayerState == nullptr)
	{
		PlayerState = GetPlayerPawn()->GetPlayerState<AStealthPlayerState>();
	}

	return PlayerState.Get();
}
