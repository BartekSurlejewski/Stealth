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
	const float DecayDelay = Profile ? Profile->LosePlayerSightGracePeriod : 1.5f;

	if (bHasPlayer && bHasPlayerLineOfSight)
	{
		LastKnownPlayerPos = Player->GetActorLocation();
		LostPlayerSightDuration = 0.0f;

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
			// Player is in sight, but is behaving legally in a permitted area -> Suspicion decays smoothly after grace period
			if (TimeSinceLastStimulus >= DecayDelay)
			{
				CurrentSuspicion -= DecayRate * DeltaTime;
			}
		}
	}
	else
	{
		LostPlayerSightDuration += DeltaTime;
		if (CurrentBehaviourState == ENpcBehaviourState::Search)
		{
			SearchDurationTimer += DeltaTime;
		}

		// Decay suspicion when out of sight after grace period
		if (TimeSinceLastStimulus >= DecayDelay)
		{
			CurrentSuspicion -= DecayRate * DeltaTime;
		}
	}

	CurrentSuspicion = FMath::Clamp(CurrentSuspicion, 0.0f, 100.0f);
	OnSuspicionChanged.Broadcast(CurrentSuspicion);

	// Update effective sight flag: direct line of sight AND (suspicion > 10, player illegal, or hostile state)
	const bool bWasEffectivelySeeing = bEffectivelySeesPlayer;
	const bool bIsIllegal = bHasPlayer && IsPlayerPerformingIllegalAction(Player);
	bEffectivelySeesPlayer = bHasPlayerLineOfSight && (CurrentSuspicion > 10.0f || bIsIllegal || AlertLevel == ENpcAlertLevel::Hostile);

	if (bWasEffectivelySeeing != bEffectivelySeesPlayer)
	{
		OnPlayerInSightChanged.Broadcast(bEffectivelySeesPlayer);
	}
}

void UNpcSuspicionComponent::EvaluateAlertState()
{
	const float AlertThreshold = Profile ? Profile->SuspicionThreshold_Alert : 75.0f;
	const float SuspiciousThreshold = 25.0f;
	const float SightLossThreshold = Profile ? Profile->LosePlayerSightGracePeriod : 2.0f;

	FGameplayTag TargetStateTag = StealthAiTags::TAG_NPC_State_Unaware;
	ENpcAlertLevel TargetAlertLevel = ENpcAlertLevel::Unaware;
	ENpcBehaviourState TargetBehaviour = ENpcBehaviourState::Routine;

	// Handle Combat state transitions
	if (CurrentBehaviourState == ENpcBehaviourState::Combat)
	{
		const AStealthPlayerCharacter* Player = GetPlayerCharacter();
		const bool bIsIllegal = Player && IsPlayerPerformingIllegalAction(Player);

		if (bHasPlayerLineOfSight && (bEffectivelySeesPlayer || bIsIllegal || CurrentSuspicion >= AlertThreshold))
		{
			// Maintain combat while player is visible and confirmed hostile/suspicious
			TargetStateTag = StealthAiTags::TAG_NPC_State_Combat;
			TargetAlertLevel = ENpcAlertLevel::Hostile;
			TargetBehaviour = ENpcBehaviourState::Combat;
		}
		else if (LostPlayerSightDuration >= SightLossThreshold || CurrentSuspicion < AlertThreshold)
		{
			// Lost sight of target during combat -> Transition to Search
			TargetStateTag = StealthAiTags::TAG_NPC_State_Search;
			TargetAlertLevel = ENpcAlertLevel::Alerted;
			TargetBehaviour = ENpcBehaviourState::Search;
			SearchDurationTimer = 0.0f;
		}
		else
		{
			// Within grace period while losing sight in combat
			TargetStateTag = StealthAiTags::TAG_NPC_State_Combat;
			TargetAlertLevel = ENpcAlertLevel::Hostile;
			TargetBehaviour = ENpcBehaviourState::Combat;
		}
	}
	// Handle Search state transitions
	else if (CurrentBehaviourState == ENpcBehaviourState::Search)
	{
		const AStealthPlayerCharacter* Player = GetPlayerCharacter();
		const bool bIsIllegal = Player && IsPlayerPerformingIllegalAction(Player);

		if (bHasPlayerLineOfSight && (bIsIllegal || CurrentSuspicion >= AlertThreshold))
		{
			// Player re-spotted during search -> Re-escalate to Combat!
			TargetStateTag = StealthAiTags::TAG_NPC_State_Combat;
			TargetAlertLevel = ENpcAlertLevel::Hostile;
			TargetBehaviour = ENpcBehaviourState::Combat;
			CurrentSuspicion = 100.0f;
		}
		else if (SearchDurationTimer >= 6.0f || CurrentSuspicion <= 0.0f)
		{
			// Search duration expired or suspicion fully decayed -> Return to Routine/Unaware
			TargetStateTag = StealthAiTags::TAG_NPC_State_Unaware;
			TargetAlertLevel = ENpcAlertLevel::Unaware;
			TargetBehaviour = ENpcBehaviourState::Routine;
			CurrentSuspicion = 0.0f;
			SearchDurationTimer = 0.0f;
		}
		else
		{
			// Remain searching
			TargetStateTag = StealthAiTags::TAG_NPC_State_Search;
			TargetAlertLevel = ENpcAlertLevel::Alerted;
			TargetBehaviour = ENpcBehaviourState::Search;
		}
	}
	// Handle non-combat / non-search states (Routine, Suspicious, Alerted)
	else
	{
		// Upward escalation
		if (CurrentSuspicion >= 100.0f || (bIsPlayerInRestrictedArea && bEffectivelySeesPlayer))
		{
			TargetStateTag = StealthAiTags::TAG_NPC_State_Combat;
			TargetAlertLevel = ENpcAlertLevel::Hostile;
			TargetBehaviour = ENpcBehaviourState::Combat;
		}
		else if (CurrentSuspicion >= AlertThreshold)
		{
			TargetStateTag = StealthAiTags::TAG_NPC_State_Alerted;
			TargetAlertLevel = ENpcAlertLevel::Alerted;
			TargetBehaviour = ENpcBehaviourState::Alerted;
		}
		else if (CurrentSuspicion >= SuspiciousThreshold)
		{
			TargetStateTag = StealthAiTags::TAG_NPC_State_Suspicious;
			TargetAlertLevel = ENpcAlertLevel::Suspicious;
			TargetBehaviour = ENpcBehaviourState::Suspicious;
		}
		else
		{
			// Downward de-escalation with hysteresis
			if (CurrentBehaviourState == ENpcBehaviourState::Alerted && CurrentSuspicion >= 50.0f)
			{
				TargetStateTag = StealthAiTags::TAG_NPC_State_Alerted;
				TargetAlertLevel = ENpcAlertLevel::Alerted;
				TargetBehaviour = ENpcBehaviourState::Alerted;
			}
			else if ((CurrentBehaviourState == ENpcBehaviourState::Suspicious || CurrentBehaviourState == ENpcBehaviourState::Alerted) && CurrentSuspicion > 5.0f)
			{
				TargetStateTag = StealthAiTags::TAG_NPC_State_Suspicious;
				TargetAlertLevel = ENpcAlertLevel::Suspicious;
				TargetBehaviour = ENpcBehaviourState::Suspicious;
			}
			else
			{
				TargetStateTag = StealthAiTags::TAG_NPC_State_Unaware;
				TargetAlertLevel = ENpcAlertLevel::Unaware;
				TargetBehaviour = ENpcBehaviourState::Routine;
			}
		}
	}

	// Only update and broadcast when an actual state or alert level change occurs
	if (AlertLevel != TargetAlertLevel || CurrentBehaviourState != TargetBehaviour)
	{
		AlertLevel = TargetAlertLevel;
		CurrentBehaviourState = TargetBehaviour;

		OnAlertLevelChanged.Broadcast(AlertLevel);
		OnBehaviourStateChanged.Broadcast(CurrentBehaviourState);
		OnAlertStateEvaluated.Broadcast(TargetStateTag, AlertLevel);
	}
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
		// Only react to non-player actors if they represent actual disturbances (dead bodies, alarms, crimes)
		const bool bIsDeadBody = Actor->ActorHasTag(FName("DeadBody"));
		const bool bIsDisturbance = Actor->ActorHasTag(FName("Disturbance")) || Actor->ActorHasTag(FName("Suspicious"));

		if (bIsDeadBody || bIsDisturbance)
		{
			LastPerceivedActor = const_cast<AActor*>(Actor);
			TimeSinceLastStimulus = 0.0f;
			AddSuspicion(bIsDeadBody ? 50.0f : 25.0f);

			if (UNpcFocusComponent* FocusComp = GetFocusComponent())
			{
				FNpcFocusTarget DisturbanceFocus;
				DisturbanceFocus.FocusLocation = Stimulus.StimulusLocation;
				DisturbanceFocus.FocusActor = const_cast<AActor*>(Actor);
				DisturbanceFocus.Duration = 5.0f;

				if (bIsDeadBody)
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
}

ENpcNoiseType UNpcSuspicionComponent::ClassifyNoiseStimulus(const FAIStimulus& Stimulus) const
{
	const FString TagStr = Stimulus.Tag.ToString();

	//TODO: Move noise classification to some other design settings, not to hardcode it
	if (Stimulus.Tag == StealthAiTags::TAG_Noise_Critical.GetTag().GetTagName() ||
		TagStr.Contains(TEXT("Explosion"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Death"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Scream"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Alarm"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("BodyFall"), ESearchCase::IgnoreCase))
	{
		return ENpcNoiseType::Critical;
	}

	if (Stimulus.Tag == StealthAiTags::TAG_Noise_Major.GetTag().GetTagName() ||
		TagStr.Contains(TEXT("Glass"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Shatter"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("DoorKick"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("DoorBreak"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("WeaponClash"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("CombatNoise"), ESearchCase::IgnoreCase))
	{
		return ENpcNoiseType::Major;
	}

	if (Stimulus.Tag == StealthAiTags::TAG_Noise_Distraction.GetTag().GetTagName() ||
		Stimulus.Tag == StealthAiTags::TAG_NPC_Focus_Noise_Distraction.GetTag().GetTagName() ||
		TagStr.Contains(TEXT("Rock"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Distraction"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Whistle"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Decoy"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Coin"), ESearchCase::IgnoreCase))
	{
		return ENpcNoiseType::Distraction;
	}

	if (Stimulus.Tag == StealthAiTags::TAG_Noise_Footstep.GetTag().GetTagName() ||
		TagStr.Contains(TEXT("Footstep"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Movement"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Sneak"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Crouch"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Walk"), ESearchCase::IgnoreCase) ||
		TagStr.Contains(TEXT("Run"), ESearchCase::IgnoreCase))
	{
		return ENpcNoiseType::Subtle;
	}

	// Fallback to loudness/strength if tag is unspecified
	if (Stimulus.Strength >= 0.9f)
	{
		return ENpcNoiseType::Critical;
	}
	if (Stimulus.Strength >= 0.65f)
	{
		return ENpcNoiseType::Major;
	}
	if (Stimulus.Strength >= 0.35f)
	{
		return ENpcNoiseType::Distraction;
	}

	return ENpcNoiseType::Subtle;
}

bool UNpcSuspicionComponent::ShouldReactToNoise(ENpcNoiseType NoiseType, const FAIStimulus& Stimulus) const
{
	// While in direct Combat with the player, NPCs do not get distracted by background noises
	if (CurrentBehaviourState == ENpcBehaviourState::Combat || AlertLevel == ENpcAlertLevel::Hostile)
	{
		return false;
	}

	// While Unaware / Routine, subtle noises (e.g. player footsteps) are ignored
	if (CurrentBehaviourState == ENpcBehaviourState::Routine || AlertLevel == ENpcAlertLevel::Unaware)
	{
		return (NoiseType != ENpcNoiseType::Subtle);
	}

	// In Suspicious, Alerted, or Search states, all noise types (including subtle footsteps) catch attention
	return true;
}

bool UNpcSuspicionComponent::OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed())
	{
		return false;
	}

	const ENpcNoiseType NoiseType = ClassifyNoiseStimulus(Stimulus);
	if (!ShouldReactToNoise(NoiseType, Stimulus))
	{
		return false;
	}

	LastHeardSoundLocation = Stimulus.StimulusLocation;
	TimeSinceLastStimulus = 0.0f;

	if (UNpcFocusComponent* FocusComp = GetFocusComponent())
	{
		FNpcFocusTarget NoiseFocus;
		NoiseFocus.FocusLocation = Stimulus.StimulusLocation;

		switch (NoiseType)
		{
		case ENpcNoiseType::Critical:
			NoiseFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Disturbance_DeadBody;
			NoiseFocus.Priority = ENpcFocusPriority::CriticalDisturbance;
			NoiseFocus.Duration = 6.0f;
			NoiseFocus.AwarenessReductionMultiplier = 0.6f;
			AddSuspicion(75.0f);
			break;

		case ENpcNoiseType::Major:
			NoiseFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Disturbance_Environment;
			NoiseFocus.Priority = ENpcFocusPriority::MajorDisturbance;
			NoiseFocus.Duration = 5.0f;
			NoiseFocus.AwarenessReductionMultiplier = 0.5f;
			AddSuspicion(35.0f);
			break;

		case ENpcNoiseType::Distraction:
			NoiseFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Noise_Distraction;
			NoiseFocus.Priority = ENpcFocusPriority::MinorDistraction;
			NoiseFocus.Duration = 4.0f;
			NoiseFocus.AwarenessReductionMultiplier = 0.4f;
			AddSuspicion(20.0f);
			break;

		case ENpcNoiseType::Subtle:
		default:
			NoiseFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Noise_Distraction;
			NoiseFocus.Priority = ENpcFocusPriority::MinorDistraction;
			NoiseFocus.Duration = 3.5f;
			NoiseFocus.AwarenessReductionMultiplier = 0.5f;
			AddSuspicion(15.0f);
			break;
		}

		FocusComp->RequestFocus(NoiseFocus);
	}

	OnNoiseHeard.Broadcast(NoiseType, Stimulus.StimulusLocation);
	return true;
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
	if (AlertLevel == NewAlertLevel)
	{
		return;
	}

	AlertLevel = NewAlertLevel;
	OnAlertLevelChanged.Broadcast(AlertLevel);

	switch (NewAlertLevel)
	{
	case ENpcAlertLevel::Hostile:
		CurrentBehaviourState = ENpcBehaviourState::Combat;
		break;
	case ENpcAlertLevel::Alerted:
		CurrentBehaviourState = ENpcBehaviourState::Alerted;
		break;
	case ENpcAlertLevel::Suspicious:
		CurrentBehaviourState = ENpcBehaviourState::Suspicious;
		break;
	case ENpcAlertLevel::Unaware:
	default:
		CurrentBehaviourState = ENpcBehaviourState::Routine;
		break;
	}

	OnBehaviourStateChanged.Broadcast(CurrentBehaviourState);
}

void UNpcSuspicionComponent::SetBehaviourState(ENpcBehaviourState NewState)
{
	if (CurrentBehaviourState == NewState)
	{
		return;
	}

	CurrentBehaviourState = NewState;
	OnBehaviourStateChanged.Broadcast(CurrentBehaviourState);

	switch (NewState)
	{
	case ENpcBehaviourState::Combat:
		AlertLevel = ENpcAlertLevel::Hostile;
		break;
	case ENpcBehaviourState::Search:
	case ENpcBehaviourState::Alerted:
		AlertLevel = ENpcAlertLevel::Alerted;
		break;
	case ENpcBehaviourState::Suspicious:
		AlertLevel = ENpcAlertLevel::Suspicious;
		break;
	case ENpcBehaviourState::Routine:
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
