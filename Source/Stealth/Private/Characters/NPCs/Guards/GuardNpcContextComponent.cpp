#include "Characters/NPCs/Guards/GuardNpcContextComponent.h"

#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/Guards/GuardNpcProfile.h"
#include "Characters/NPCs/Guards/NpcPatrolComponent.h"
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
}

//TODO: Add changing to suspicious state when guard sees player doing something illegal, 
// then to Alert state when player does it again in a given time period
void UGuardNpcContextComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (BehaviourState == EGuardBehaviourState::Search && SearchTimer > 0.f)
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


void UGuardNpcContextComponent::ForceAlert(FVector AtLocation)
{
	LastKnownPlayerPos = AtLocation;
	SendGuardEvent(AlertThresholdMetTag);
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
		LastKnownPlayerPos = Stimulus.StimulusLocation;
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

	NpcAiController->SetFocus(GetPlayerPawn());
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

float UGuardNpcContextComponent::GetSuspicionModifier(AActor* Target) const
{
	if (!Target || !Profile)
	{
		return 1.f;
	}
	float Modifier = 1.f;
	ACharacter* C = Cast<ACharacter>(Target);

	if (C && C->GetCharacterMovement()->IsCrouching())
	{
		Modifier *= Profile->CrouchSuspicionMultiplier;
	}

	return Modifier;
}

APawn* UGuardNpcContextComponent::GetPlayerPawn()
{
	if (PlayerPawn == nullptr)
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	return PlayerPawn.Get();
}
