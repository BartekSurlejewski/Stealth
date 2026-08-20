#include "Characters/NPCs/AI/Suspicion/NpcSuspicionComponent.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcCharacter.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/World.h"
#include "Exposure/PlayerExposureSubsystem.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/StealthMessages.h"
#include "Perception/AIPerceptionTypes.h"
#include "Stealth/Stealth.h"

UNpcSuspicionComponent::UNpcSuspicionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.05f; // 20 Hz
}

void UNpcSuspicionComponent::BeginPlay()
{
	Super::BeginPlay();

	GetAiController();
	GetFocusComponent();

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	PlayerInRestrictedAreaListenerHandle = MsgSubsystem.RegisterListener<FBooleanMessage>(
		StealthMessageChannels::TAG_Message_Player_IsInRestrictedAreaChanged,
		this,
		&UNpcSuspicionComponent::OnPlayerInRestrictedAreaChanged
	);
}

void UNpcSuspicionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PlayerInRestrictedAreaListenerHandle.IsValid())
	{
		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
		MsgSubsystem.UnregisterListener(PlayerInRestrictedAreaListenerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UNpcSuspicionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastStimulus += DeltaTime;
	UpdateSuspicion(DeltaTime);
	EvaluateAlertState();
}

ANpcAiController* UNpcSuspicionComponent::GetAiController() const
{
	if (!CachedController.IsValid())
	{
		if (ANpcAiController* Controller = Cast<ANpcAiController>(GetOwner()))
		{
			CachedController = Controller;
		}
		else if (const APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			CachedController = Cast<ANpcAiController>(Pawn->GetController());
		}
	}
	return CachedController.Get();
}

UNpcFocusComponent* UNpcSuspicionComponent::GetFocusComponent() const
{
	if (!CachedFocusComponent.IsValid())
	{
		if (GetOwner())
		{
			CachedFocusComponent = GetOwner()->FindComponentByClass<UNpcFocusComponent>();
		}
		if (!CachedFocusComponent.IsValid())
		{
			if (const ANpcAiController* Controller = GetAiController())
			{
				CachedFocusComponent = Controller->FindComponentByClass<UNpcFocusComponent>();
			}
		}
	}
	return CachedFocusComponent.Get();
}

AStealthPlayerCharacter* UNpcSuspicionComponent::GetPlayerCharacter() const
{
	const UCharactersRegistrySubsystem* CharactersRegistry = UCharactersRegistrySubsystem::Get(this);
	if (!CharactersRegistry)
	{
		return nullptr;
	}

	return CharactersRegistry->GetPlayerCharacter();
}

bool UNpcSuspicionComponent::IsPlayerPerformingIllegalAction(const AStealthPlayerCharacter* Player) const
{
	if (!Player)
	{
		return false;
	}

	// 1. Check trespassing in restricted area
	if (bIsPlayerInRestrictedArea)
	{
		return true;
	}

	// 2. Check GAS gameplay tags for illegal or suspicious states
	if (const IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Player))
	{
		if (const UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent())
		{
			if (ASC->HasMatchingGameplayTag(StealthAiTags::TAG_Player_State_Illegal) ||
				ASC->HasMatchingGameplayTag(StealthAiTags::TAG_Player_State_Trespassing))
			{
				return true;
			}
		}
	}

	return false;
}

void UNpcSuspicionComponent::UpdateSuspicion(float DeltaTime)
{
	const AStealthPlayerCharacter* Player = GetPlayerCharacter();
	const bool bHasPlayer = (Player != nullptr);
	const float DecayRate = Profile ? Profile->SuspicionDecayPerSecond : 8.0f;

	if (bHasPlayer && bHasPlayerLineOfSight)
	{
		LastKnownPlayerPos = Player->GetActorLocation();

		// Accumulate suspicion ONLY if player is doing something illegal
		if (IsPlayerPerformingIllegalAction(Player))
		{
			float GainRate = Profile ? Profile->SuspicionGainPerSecond_Sight : 40.0f;
			if (!IsLookingDirectlyAtPlayer(Player))
			{
				GainRate = Profile ? Profile->SuspicionGainPerSecond_Peripheral : 15.0f;
			}

			// Apply focus awareness reduction multiplier (tunnel vision / distracted state)
			if (const UNpcFocusComponent* FocusComp = GetFocusComponent())
			{
				GainRate *= FocusComp->GetAwarenessMultiplier();
			}

			// Always see player within close range
			const float Dist = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
			const float AlwaysSeeRange = Profile ? Profile->AlwaysSeePlayerRange : 100.0f;
			const float EffectiveExposure = (Dist <= AlwaysSeeRange) ? 1.0f : CalculatePlayerExposureMultiplier();

			CurrentSuspicion += GainRate * EffectiveExposure * DeltaTime;
			TimeSinceLastStimulus = 0.0f;
		}
		else
		{
			// Player is in sight, but is behaving legally in a permitted area -> Suspicion decays
			CurrentSuspicion -= DecayRate * DeltaTime;
		}
	}
	else
	{
		// Decay suspicion when out of sight
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

void UNpcSuspicionComponent::EvaluateAlertState()
{
	const float AlertThreshold = Profile ? Profile->SuspicionThreshold_Alert : 100.0f;

	FGameplayTag TargetStateTag = StealthAiTags::TAG_NPC_State_Unaware;

	if (CurrentSuspicion >= 100.0f || (bIsPlayerInRestrictedArea && bEffectivelySeesPlayer))
	{
		TargetStateTag = StealthAiTags::TAG_NPC_State_Combat;
		AlertLevel = ENpcAlertLevel::Hostile;
		CurrentBehaviourState = EGuardBehaviourState::Alarm;
	}
	else if (CurrentSuspicion >= AlertThreshold)
	{
		TargetStateTag = StealthAiTags::TAG_NPC_State_Alerted;
		AlertLevel = ENpcAlertLevel::Alerted;
		CurrentBehaviourState = EGuardBehaviourState::Alerted;
	}
	else if (CurrentSuspicion > 5.0f)
	{
		TargetStateTag = StealthAiTags::TAG_NPC_State_Suspicious;
		AlertLevel = ENpcAlertLevel::Suspicious;
		CurrentBehaviourState = EGuardBehaviourState::Suspicious;
	}
	else
	{
		TargetStateTag = StealthAiTags::TAG_NPC_State_Unaware;
		AlertLevel = ENpcAlertLevel::Unaware;
		CurrentBehaviourState = EGuardBehaviourState::Patrol;
	}

	OnAlertStateEvaluated.Broadcast(TargetStateTag, AlertLevel);
}

float UNpcSuspicionComponent::CalculatePlayerExposureMultiplier() const
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

bool UNpcSuspicionComponent::IsLookingDirectlyAtPlayer(const AStealthPlayerCharacter* Player) const
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

void UNpcSuspicionComponent::OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus)
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

			if (IsPlayerPerformingIllegalAction(GetPlayerCharacter()) || CurrentSuspicion > 50.0f)
			{
				if (UNpcFocusComponent* FocusComp = GetFocusComponent())
				{
					FNpcFocusTarget PlayerFocus;
					PlayerFocus.FocusTag = (AlertLevel == ENpcAlertLevel::Hostile)
						? StealthAiTags::TAG_NPC_Focus_Player_Hostile
						: StealthAiTags::TAG_NPC_Focus_Player_Suspicious;
					PlayerFocus.Priority = (AlertLevel == ENpcAlertLevel::Hostile)
						? ENpcFocusPriority::CombatTarget
						: ENpcFocusPriority::SuspiciousPlayer;
					PlayerFocus.FocusActor = const_cast<AActor*>(Actor);
					PlayerFocus.FocusLocation = Actor->GetActorLocation();
					PlayerFocus.Duration = 0.0f; // Continuous tracking while in sight
					PlayerFocus.AwarenessReductionMultiplier = 1.0f;

					FocusComp->RequestFocus(PlayerFocus);
				}
			}
		}
	}
	else if (Stimulus.WasSuccessfullySensed())
	{
		// Non-player stimulus (e.g., dead bodies, suspicious disturbances)
		LastPerceivedActor = const_cast<AActor*>(Actor);
		TimeSinceLastStimulus = 0.0f;
		AddSuspicion(25.0f);

		if (UNpcFocusComponent* FocusComp = GetFocusComponent())
		{
			FNpcFocusTarget DisturbanceFocus;
			DisturbanceFocus.FocusLocation = Stimulus.StimulusLocation;
			DisturbanceFocus.FocusActor = const_cast<AActor*>(Actor);
			DisturbanceFocus.Duration = 5.0f;

			if (Actor->ActorHasTag(FName("DeadBody")))
			{
				DisturbanceFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Disturbance_DeadBody;
				DisturbanceFocus.Priority = ENpcFocusPriority::CriticalDisturbance;
			}
			else
			{
				DisturbanceFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Disturbance_Environment;
				DisturbanceFocus.Priority = ENpcFocusPriority::MajorDisturbance;
			}

			FocusComp->RequestFocus(DisturbanceFocus);
		}
	}
}

void UNpcSuspicionComponent::OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	LastHeardSoundLocation = Stimulus.StimulusLocation;
	TimeSinceLastStimulus = 0.0f;

	if (UNpcFocusComponent* FocusComp = GetFocusComponent())
	{
		FNpcFocusTarget NoiseFocus;
		NoiseFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Noise_Distraction;
		NoiseFocus.Priority = ENpcFocusPriority::MinorDistraction;
		NoiseFocus.FocusLocation = Stimulus.StimulusLocation;
		NoiseFocus.Duration = 4.0f;
		NoiseFocus.AwarenessReductionMultiplier = 0.4f;

		FocusComp->RequestFocus(NoiseFocus);
	}

	// Scale suspicion by sound strength
	const float SuspicionGain = FMath::Clamp(Stimulus.Strength * 30.0f, 10.0f, 60.0f);
	AddSuspicion(SuspicionGain);
}

void UNpcSuspicionComponent::HandleCrimeReported(const FAiCrimeEventPayload& CrimePayload)
{
	LastHeardSoundLocation = CrimePayload.CrimeLocation;
	LastKnownPlayerPos = CrimePayload.CrimeLocation;
	TimeSinceLastStimulus = 0.0f;

	if (CrimePayload.bIsPrimaryInvestigator)
	{
		AddSuspicion(100.0f);
		SetAlertLevel(ENpcAlertLevel::Hostile);
	}
	else
	{
		AddSuspicion(50.0f);
		SetAlertLevel(ENpcAlertLevel::Alerted);
	}
}

void UNpcSuspicionComponent::AddSuspicion(float Amount)
{
	CurrentSuspicion = FMath::Clamp(CurrentSuspicion + Amount, 0.0f, 100.0f);
	OnSuspicionChanged.Broadcast(CurrentSuspicion);
	EvaluateAlertState();
}

void UNpcSuspicionComponent::SetAlertLevel(ENpcAlertLevel NewAlertLevel)
{
	AlertLevel = NewAlertLevel;
	OnAlertLevelChanged.Broadcast(AlertLevel);

	switch (NewAlertLevel)
	{
	case ENpcAlertLevel::Hostile:
		CurrentBehaviourState = EGuardBehaviourState::Alarm;
		break;
	case ENpcAlertLevel::Alerted:
		CurrentBehaviourState = EGuardBehaviourState::Alerted;
		break;
	case ENpcAlertLevel::Suspicious:
		CurrentBehaviourState = EGuardBehaviourState::Suspicious;
		break;
	case ENpcAlertLevel::Unaware:
	default:
		CurrentBehaviourState = EGuardBehaviourState::Patrol;
		break;
	}

	OnBehaviourStateChanged.Broadcast(CurrentBehaviourState);
}

void UNpcSuspicionComponent::SetBehaviourState(EGuardBehaviourState NewState)
{
	CurrentBehaviourState = NewState;
	OnBehaviourStateChanged.Broadcast(CurrentBehaviourState);

	switch (NewState)
	{
	case EGuardBehaviourState::Alarm:
		AlertLevel = ENpcAlertLevel::Hostile;
		break;
	case EGuardBehaviourState::Search:
	case EGuardBehaviourState::Alerted:
		AlertLevel = ENpcAlertLevel::Alerted;
		break;
	case EGuardBehaviourState::Suspicious:
		AlertLevel = ENpcAlertLevel::Suspicious;
		break;
	case EGuardBehaviourState::Patrol:
	default:
		AlertLevel = ENpcAlertLevel::Unaware;
		break;
	}

	OnAlertLevelChanged.Broadcast(AlertLevel);
}

void UNpcSuspicionComponent::OnPlayerInRestrictedAreaChanged(FGameplayTag Channel, const FBooleanMessage& Message)
{
	bIsPlayerInRestrictedArea = Message.bValue;

	if (bIsPlayerInRestrictedArea && bEffectivelySeesPlayer)
	{
		AddSuspicion(50.0f);
	}
}
