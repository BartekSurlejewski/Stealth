#include "Characters/NPCs/Guards/GuardNpcContextComponent.h"

#include "Characters/NPCs/Guards/GuardNpcProfile.h"
#include "Characters/NPCs/Guards/NpcPatrolComponent.h"
#include "Components/StateTreeComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionTypes.h"


class UStateTreeComponent;

void UGuardNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();

	PatrolComponent = GetOwner()->FindComponentByClass<UNpcPatrolComponent>();
}


void UGuardNpcContextComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	TickSuspicion(DeltaTime);

	if (AlertLevel == EGuardAlertLevel::Search && SearchTimer > 0.f)
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

void UGuardNpcContextComponent::ForceAlert(FVector AtLocation)
{
	LastKnownPlayerPos = AtLocation;
	AlertLevel = EGuardAlertLevel::Alerted;
	if (Profile) SuspicionLevel = Profile->SuspicionThreshold_Alert;
	SendGuardEvent(AlertThresholdMetTag);
}

void UGuardNpcContextComponent::BeginSearch()
{
	AlertLevel = EGuardAlertLevel::Search;
	SearchTimer = Profile ? Profile->SearchDuration : 20.f;
}

void UGuardNpcContextComponent::OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus, float ExposureMultiplier)
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (Actor != Player)
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
}

void UGuardNpcContextComponent::OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed() || !Profile)
	{
		return;
	}

	LastHeardSoundLocation = Stimulus.StimulusLocation;
	SuspicionLevel = FMath::Min(Profile->SuspicionThreshold_Alert,
	                            SuspicionLevel + Stimulus.Strength * 50.f);

	if (AlertLevel <= EGuardAlertLevel::Suspicious)
	{
		LastKnownPlayerPos = Stimulus.StimulusLocation;
	}

	UpdateAlertLevel();
}

void UGuardNpcContextComponent::TickSuspicion(float DeltaTime)
{
	if (!Profile)
	{
		return;
	}

	float Delta = 0.f;

	if (bPlayerInDirectSight)
	{
		Delta += Profile->SuspicionGainPerSecond_Sight
			* GetSuspicionModifier(UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
			* DeltaTime;
	}
	else if (bPlayerInPeripheralSight)
	{
		Delta += Profile->SuspicionGainPerSecond_Peripheral
			* GetSuspicionModifier(UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
			* DeltaTime;
	}
	else if (AlertLevel <= EGuardAlertLevel::Suspicious)
	{
		Delta -= Profile->SuspicionDecayPerSecond * DeltaTime;
	}

	SuspicionLevel = FMath::Clamp(SuspicionLevel + Delta,
	                              0.f, Profile->SuspicionThreshold_Alert);

	UpdateAlertLevel();
}

void UGuardNpcContextComponent::UpdateAlertLevel()
{
	if (!Profile || GlobalAlarmLevel >= 2 || AlertLevel == EGuardAlertLevel::Alarm || AlertLevel >= EGuardAlertLevel::Alerted)
	{
		return;
	}

	if (SuspicionLevel >= Profile->SuspicionThreshold_Alert && AlertLevel != EGuardAlertLevel::Alerted)
	{
		AlertLevel = EGuardAlertLevel::Alerted;
		SendGuardEvent(AlertThresholdMetTag);
	}
	else if (SuspicionLevel > 0.f)
	{
		AlertLevel = EGuardAlertLevel::Suspicious;
	}
	else
	{
		AlertLevel = EGuardAlertLevel::Patrol;
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
		if (AlertLevel <= EGuardAlertLevel::Suspicious)
		{
			LastKnownPlayerPos = SourceLocation;
		}

		AlertLevel = EGuardAlertLevel::Alarm;
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
