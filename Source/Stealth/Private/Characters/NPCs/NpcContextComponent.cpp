#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Engine/World.h"
#include "Exposure/PlayerExposureSubsystem.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/StealthMessages.h"
#include "Perception/AIPerceptionTypes.h"
#include "Stealth/Stealth.h"

UNpcContextComponent::UNpcContextComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.05f; // 20 Hz for smooth suspicion accumulation
}

void UNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();

	StateTreeComponent = GetOwner()->FindComponentByClass<UStateTreeAIComponent>();
	NpcAiController = Cast<ANpcAiController>(GetOwner());

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	PlayerInRestrictedAreaListenerHandle = MsgSubsystem.RegisterListener<FBooleanMessage>(
		StealthMessageChannels::TAG_Message_Player_IsInRestrictedAreaChanged,
		this,
		&UNpcContextComponent::OnPlayerInRestrictedAreaChanged
	);
}

void UNpcContextComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PlayerInRestrictedAreaListenerHandle.IsValid())
	{
		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
		MsgSubsystem.UnregisterListener(PlayerInRestrictedAreaListenerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UNpcContextComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastStimulus += DeltaTime;
	UpdateSuspicion(DeltaTime);
	EvaluateAlertState();
}

void UNpcContextComponent::UpdateSuspicion(float DeltaTime)
{
	const AStealthPlayerCharacter* Player = GetPlayerCharacter();
	const bool bHasPlayer = (Player != nullptr);

	if (bHasPlayer && bHasPlayerLineOfSight)
	{
		LastKnownPlayerPos = Player->GetActorLocation();

		float GainRate = Profile ? Profile->SuspicionGainPerSecond_Sight : 40.0f;
		if (!IsLookingDirectlyAtPlayer(Player))
		{
			GainRate = Profile ? Profile->SuspicionGainPerSecond_Peripheral : 15.0f;
		}

		const float ExposureMultiplier = CalculatePlayerExposureMultiplier();

		// Always see player within close range
		const float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
		const float AlwaysSeeRange = Profile ? Profile->AlwaysSeePlayerRange : 100.0f;
		const float EffectiveExposure = (Dist <= AlwaysSeeRange) ? 1.0f : ExposureMultiplier;

		CurrentSuspicion += GainRate * EffectiveExposure * DeltaTime;
		TimeSinceLastStimulus = 0.0f;
	}
	else
	{
		// Decay suspicion when out of sight
		const float DecayRate = Profile ? Profile->SuspicionDecayPerSecond : 8.0f;
		CurrentSuspicion -= DecayRate * DeltaTime;
	}

	CurrentSuspicion = FMath::Clamp(CurrentSuspicion, 0.0f, 100.0f);
	OnSuspicionChanged.Broadcast(CurrentSuspicion);

	// Update effective sight flag based on threshold
	const bool bWasEffectivelySeeing = bEffectivelySeesPlayer;
	bEffectivelySeesPlayer = bHasPlayerLineOfSight && (CurrentSuspicion > 10.0f);

	if (bWasEffectivelySeeing != bEffectivelySeesPlayer)
	{
		OnPlayerInSightChanged.Broadcast(bEffectivelySeesPlayer);
	}
}

void UNpcContextComponent::EvaluateAlertState()
{
	const float AlertThreshold = Profile ? Profile->SuspicionThreshold_Alert : 100.0f;

	ENpcAlertLevel NewAlertLevel = ENpcAlertLevel::Unaware;
	EGuardBehaviourState NewBehaviour = EGuardBehaviourState::Patrol;

	if (CurrentSuspicion >= 100.0f || (bIsPlayerInRestrictedArea && bEffectivelySeesPlayer))
	{
		NewAlertLevel = ENpcAlertLevel::Hostile;
		NewBehaviour = EGuardBehaviourState::Alarm;
	}
	else if (CurrentSuspicion >= AlertThreshold)
	{
		NewAlertLevel = ENpcAlertLevel::Alerted;
		NewBehaviour = EGuardBehaviourState::Alerted;
	}
	else if (CurrentSuspicion > 5.0f)
	{
		NewAlertLevel = ENpcAlertLevel::Suspicious;
		NewBehaviour = EGuardBehaviourState::Suspicious;
	}

	if (AlertLevel != NewAlertLevel)
	{
		SetAlertLevel(NewAlertLevel);
	}

	if (CurrentBehaviourState != NewBehaviour)
	{
		SetBehaviourState(NewBehaviour);
	}
}

float UNpcContextComponent::CalculatePlayerExposureMultiplier() const
{
	float Multiplier = 1.0f;

	if (const UPlayerExposureSubsystem* ExposureSubsystem = UPlayerExposureSubsystem::Get(this))
	{
		Multiplier = ExposureSubsystem->GetCurrentTotalExposure();
	}

	if (const AStealthPlayerCharacter* Player = GetPlayerCharacter())
	{
		if (Player->bIsCrouched && Profile)
		{
			Multiplier *= Profile->CrouchSuspicionMultiplier;
		}
	}

	return FMath::Clamp(Multiplier, 0.05f, 1.0f);
}

bool UNpcContextComponent::IsLookingDirectlyAtPlayer(const AStealthPlayerCharacter* Player) const
{
	if (!Player || !GetOwner())
	{
		return false;
	}

	const FVector Forward = GetOwner()->GetActorForwardVector();
	const FVector DirToPlayer = (Player->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
	const float Dot = FVector::DotProduct(Forward, DirToPlayer);

	return Dot >= 0.707f; // Within ~45 degrees of center view
}

void UNpcContextComponent::OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Actor)
	{
		return;
	}

	if (Actor == GetPlayerCharacter())
	{
		bHasPlayerLineOfSight = (Stimulus.WasSuccessfullySensed() && Stimulus.Strength >= 0.05f);
		if (bHasPlayerLineOfSight)
		{
			LastKnownPlayerPos = Actor->GetActorLocation();
			TimeSinceLastStimulus = 0.0f;
		}
	}
	else if (Stimulus.WasSuccessfullySensed())
	{
		// Non-player stimulus (e.g., dead bodies, suspicious disturbances)
		LastPerceivedActor = const_cast<AActor*>(Actor);
		TimeSinceLastStimulus = 0.0f;
		AddSuspicion(25.0f);
	}
}

void UNpcContextComponent::OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	LastHeardSoundLocation = Stimulus.StimulusLocation;
	TimeSinceLastStimulus = 0.0f;

	// Scale suspicion by sound strength
	const float SuspicionGain = FMath::Clamp(Stimulus.Strength * 30.0f, 10.0f, 60.0f);
	AddSuspicion(SuspicionGain);

	if (InvestigateActivityTag.IsValid())
	{
		SendStateTreeEvent(InvestigateActivityTag);
	}
}

void UNpcContextComponent::HandleCrimeReported(const FAiCrimeEventPayload& CrimePayload)
{
	LastHeardSoundLocation = CrimePayload.CrimeLocation;
	LastKnownPlayerPos = CrimePayload.CrimeLocation;
	TimeSinceLastStimulus = 0.0f;

	if (CrimePayload.bIsPrimaryInvestigator)
	{
		AddSuspicion(100.0f);
		SetAlertLevel(ENpcAlertLevel::Hostile);
		if (AlarmActivityTag.IsValid())
		{
			SendStateTreeEvent(AlarmActivityTag);
		}
	}
	else
	{
		AddSuspicion(50.0f);
		SetAlertLevel(ENpcAlertLevel::Alerted);
		if (InvestigateActivityTag.IsValid())
		{
			SendStateTreeEvent(InvestigateActivityTag);
		}
	}
}

void UNpcContextComponent::AddSuspicion(float Amount)
{
	CurrentSuspicion = FMath::Clamp(CurrentSuspicion + Amount, 0.0f, 100.0f);
	OnSuspicionChanged.Broadcast(CurrentSuspicion);
	EvaluateAlertState();
}

void UNpcContextComponent::SetAlertLevel(ENpcAlertLevel NewAlertLevel)
{
	if (AlertLevel != NewAlertLevel)
	{
		AlertLevel = NewAlertLevel;
		OnAlertLevelChanged.Broadcast(AlertLevel);

		switch (AlertLevel)
		{
		case ENpcAlertLevel::Suspicious:
			if (SuspiciousActivityTag.IsValid()) SendStateTreeEvent(SuspiciousActivityTag);
			break;
		case ENpcAlertLevel::Alerted:
			if (AlertedActivityTag.IsValid()) SendStateTreeEvent(AlertedActivityTag);
			break;
		case ENpcAlertLevel::Hostile:
			if (AlarmActivityTag.IsValid()) SendStateTreeEvent(AlarmActivityTag);
			break;
		default:
			break;
		}
	}
}

void UNpcContextComponent::SetBehaviourState(EGuardBehaviourState NewState)
{
	if (CurrentBehaviourState != NewState)
	{
		CurrentBehaviourState = NewState;
		OnBehaviourStateChanged.Broadcast(CurrentBehaviourState);
	}
}

void UNpcContextComponent::SendStateTreeEvent(const FGameplayTag& Tag) const
{
	if (StateTreeComponent && Tag.IsValid())
	{
		StateTreeComponent->SendStateTreeEvent(FStateTreeEvent(Tag));
	}
}

AStealthPlayerCharacter* UNpcContextComponent::GetPlayerCharacter() const
{
	const UCharactersRegistrySubsystem* CharactersRegistry = UCharactersRegistrySubsystem::Get(this);
	if (!CharactersRegistry)
	{
		return nullptr;
	}

	return CharactersRegistry->GetPlayerCharacter();
}

void UNpcContextComponent::OnPlayerInRestrictedAreaChanged(FGameplayTag Channel, const FBooleanMessage& Message)
{
	bIsPlayerInRestrictedArea = Message.bValue;

	if (bIsPlayerInRestrictedArea && bEffectivelySeesPlayer)
	{
		AddSuspicion(50.0f);
		if (SuspiciousActivityTag.IsValid())
		{
			SendStateTreeEvent(SuspiciousActivityTag);
		}
	}
}
