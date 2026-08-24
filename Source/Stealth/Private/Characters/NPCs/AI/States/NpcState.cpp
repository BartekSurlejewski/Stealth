#include "Characters/NPCs/AI/States/NpcState.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/AI/Suspicion/NpcSuspicionComponent.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UNpcStateProfile::ApplyToNpc_Implementation(UNpcContextComponent* Context)
{
	if (!Context)
	{
		return;
	}

	AActor* OwnerActor = Context->GetOwner();
	ACharacter* Character = Cast<ACharacter>(OwnerActor);
	if (!Character && OwnerActor)
	{
		if (const AController* Controller = Cast<AController>(OwnerActor))
		{
			Character = Cast<ACharacter>(Controller->GetPawn());
		}
	}

	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = MaxWalkSpeed;
	}
}

UNpcState::UNpcState()
{
	StateProfile = CreateDefaultSubobject<UNpcStateProfile>(TEXT("DefaultStateProfile"));
}

void UNpcState::OnEnterState_Implementation(UNpcContextComponent* Context, UNpcState* PreviousState)
{
	// Note: State profile application is handled once centrally by the state transition router (TransitionToState)
	// to prevent clobbering per-NPC profile overrides. Subclasses can add state-entry logic here.
}

void UNpcState::OnExitState_Implementation(UNpcContextComponent* Context, UNpcState* NextState) {}

void UNpcState::OnTickState_Implementation(UNpcContextComponent* Context, float DeltaTime) {}

float UNpcState::GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const
{
	if (Ctx.bIsFocusingOnPlayer && Ctx.bHasLineOfSight && Ctx.bIsPlayerPerformingIllegalAction)
	{
		const float BaseRate = Ctx.bIsLookingDirectlyAtPlayer ? Ctx.BaseGainRateSight : Ctx.BaseGainRatePeripheral;
		const float ProfileGainMult = StateProfile ? StateProfile->SuspicionGainMultiplier : 1.0f;
		return BaseRate * ProfileGainMult * Ctx.FocusAwarenessMultiplier * Ctx.PlayerExposureMultiplier * Ctx.DeltaTime;
	}

	const float ProfileDecayMult = StateProfile ? StateProfile->SuspicionDecayMultiplier : 1.0f;
	return -Ctx.BaseDecayRate * ProfileDecayMult * Ctx.DeltaTime;
}

FGameplayTag UNpcState::EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const
{
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState::HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus)
{
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState::HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType)
{
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState::HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload)
{
	return FGameplayTag::EmptyTag;
}

bool UNpcState::CanTransitionTo_Implementation(UNpcContextComponent* Context, const UNpcState* CandidateState) const
{
	return true;
}

void UNpcState::ApplyStateProfile(UNpcContextComponent* Context)
{
	if (StateProfile && Context)
	{
		StateProfile->ApplyToNpc(Context);
	}
}

// ---------------------------------------------------------------------------
// UNpcState_Unaware
// ---------------------------------------------------------------------------

UNpcState_Unaware::UNpcState_Unaware()
{
	StateTag = StealthAiTags::TAG_NPC_State_Unaware;
	AlertLevel = ENpcAlertLevel::Unaware;
	BehaviourState = ENpcBehaviourState::Routine;
	Priority = 0;

	if (StateProfile)
	{
		StateProfile->MaxWalkSpeed = 200.0f;
		StateProfile->DefaultFocusPriority = ENpcFocusPriority::None;
		StateProfile->SuspicionGainMultiplier = 1.0f;
		StateProfile->SuspicionDecayMultiplier = 1.0f;
	}
}

float UNpcState_Unaware::GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const
{
	if (Ctx.bIsFocusingOnPlayer && Ctx.bHasLineOfSight && Ctx.bIsPlayerPerformingIllegalAction)
	{
		const float BaseRate = Ctx.bIsLookingDirectlyAtPlayer ? Ctx.BaseGainRateSight : Ctx.BaseGainRatePeripheral;
		const float ProfileGainMult = StateProfile ? StateProfile->SuspicionGainMultiplier : 1.0f;
		return BaseRate * ProfileGainMult * Ctx.FocusAwarenessMultiplier * Ctx.PlayerExposureMultiplier * Ctx.DeltaTime;
	}

	const float ProfileDecayMult = StateProfile ? StateProfile->SuspicionDecayMultiplier : 1.0f;
	return -Ctx.BaseDecayRate * ProfileDecayMult * Ctx.DeltaTime;
}

FGameplayTag UNpcState_Unaware::EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const
{
	if (Ctx.CurrentSuspicion >= Ctx.AlertThreshold)
	{
		return StealthAiTags::TAG_NPC_State_Alerted;
	}
	if (Ctx.CurrentSuspicion >= Ctx.SuspiciousThreshold)
	{
		return StealthAiTags::TAG_NPC_State_Suspicious;
	}
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Unaware::HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	if (SeenActor == Context->GetPlayerCharacter())
	{
		const bool bIllegal = Context->IsPlayerPerformingIllegalAction();
		if (!bIllegal)
		{
			return FGameplayTag::EmptyTag;
		}

		FNpcFocusTarget PlayerFocus;
		PlayerFocus.FocusActor = SeenActor;
		PlayerFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Player_Hostile;
		PlayerFocus.Priority = ENpcFocusPriority::CombatTarget;
		PlayerFocus.Duration = 0.0f;

		// Focus arbitration gate: If occupied with equal or higher priority, ignore the player
		if (!Context->RequestFocus(PlayerFocus))
		{
			return FGameplayTag::EmptyTag;
		}

		Context->AddSuspicion(100.0f);
		return StealthAiTags::TAG_NPC_State_Combat;
	}

	// Environmental disturbance seen
	FNpcFocusTarget DisturbanceFocus;
	DisturbanceFocus.FocusActor = SeenActor;
	DisturbanceFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Disturbance_Environment;
	DisturbanceFocus.Priority = ENpcFocusPriority::MajorDisturbance;
	DisturbanceFocus.Duration = 5.0f;

	if (Context->RequestFocus(DisturbanceFocus))
	{
		Context->AddSuspicion(25.0f);
		return StealthAiTags::TAG_NPC_State_Alerted;
	}

	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Unaware::HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	ENpcFocusPriority FocusPriority = ENpcFocusPriority::MinorDistraction;
	float SuspicionToAdd = 25.0f;

	switch (NoiseType)
	{
	case ENpcNoiseType::Critical:
		FocusPriority = ENpcFocusPriority::CriticalDisturbance;
		SuspicionToAdd = 75.0f;
		break;
	case ENpcNoiseType::Major:
		FocusPriority = ENpcFocusPriority::MajorDisturbance;
		SuspicionToAdd = 40.0f;
		break;
	case ENpcNoiseType::Distraction:
	case ENpcNoiseType::Subtle:
	default:
		FocusPriority = ENpcFocusPriority::MinorDistraction;
		SuspicionToAdd = 25.0f;
		break;
	}

	FNpcFocusTarget SoundFocus;
	SoundFocus.FocusLocation = Stimulus.StimulusLocation;
	SoundFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Noise_Distraction;
	SoundFocus.Priority = FocusPriority;
	SoundFocus.Duration = 4.0f;

	if (!Context->RequestFocus(SoundFocus))
	{
		return FGameplayTag::EmptyTag;
	}

	Context->AddSuspicion(SuspicionToAdd);
	return (NoiseType == ENpcNoiseType::Subtle && Context->GetSuspicion() < 25.0f) ? StealthAiTags::TAG_NPC_State_Suspicious : StealthAiTags::TAG_NPC_State_Alerted;
}

FGameplayTag UNpcState_Unaware::HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	FNpcFocusTarget CrimeFocus;
	CrimeFocus.FocusLocation = Payload.CrimeLocation;
	CrimeFocus.Priority = Payload.bIsPrimaryInvestigator ? ENpcFocusPriority::CombatTarget : ENpcFocusPriority::MajorDisturbance;
	CrimeFocus.Duration = Payload.bIsPrimaryInvestigator ? 0.0f : 8.0f;

	Context->RequestFocus(CrimeFocus);
	Context->AddSuspicion(Payload.bIsPrimaryInvestigator ? 100.0f : 50.0f);
	return Payload.bIsPrimaryInvestigator ? StealthAiTags::TAG_NPC_State_Combat : StealthAiTags::TAG_NPC_State_Alerted;
}

// ---------------------------------------------------------------------------
// UNpcState_Suspicious
// ---------------------------------------------------------------------------

UNpcState_Suspicious::UNpcState_Suspicious()
{
	StateTag = StealthAiTags::TAG_NPC_State_Suspicious;
	AlertLevel = ENpcAlertLevel::Suspicious;
	BehaviourState = ENpcBehaviourState::Suspicious;
	Priority = 20;

	if (StateProfile)
	{
		StateProfile->MaxWalkSpeed = 280.0f;
		StateProfile->DefaultFocusPriority = ENpcFocusPriority::SuspiciousPlayer;
		StateProfile->SuspicionGainMultiplier = 1.2f;
		StateProfile->SuspicionDecayMultiplier = 0.8f;
	}
}

float UNpcState_Suspicious::GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const
{
	if (Ctx.bIsFocusingOnPlayer && Ctx.bHasLineOfSight && Ctx.bIsPlayerPerformingIllegalAction)
	{
		const float BaseRate = Ctx.bIsLookingDirectlyAtPlayer ? Ctx.BaseGainRateSight : Ctx.BaseGainRatePeripheral;
		const float ProfileGainMult = StateProfile ? StateProfile->SuspicionGainMultiplier : 1.2f;
		return BaseRate * ProfileGainMult * Ctx.FocusAwarenessMultiplier * Ctx.PlayerExposureMultiplier * Ctx.DeltaTime;
	}

	const float ProfileDecayMult = StateProfile ? StateProfile->SuspicionDecayMultiplier : 0.8f;
	return -Ctx.BaseDecayRate * ProfileDecayMult * Ctx.DeltaTime;
}

FGameplayTag UNpcState_Suspicious::EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const
{
	if (Ctx.bHasLineOfSight && Ctx.bIsPlayerPerformingIllegalAction)
	{
		return StealthAiTags::TAG_NPC_State_Combat;
	}
	if (Ctx.CurrentSuspicion >= Ctx.AlertThreshold)
	{
		return StealthAiTags::TAG_NPC_State_Alerted;
	}
	if (Ctx.CurrentSuspicion <= 0.0f)
	{
		return StealthAiTags::TAG_NPC_State_Unaware;
	}
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Suspicious::HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	if (SeenActor == Context->GetPlayerCharacter())
	{
		const bool bIllegal = Context->IsPlayerPerformingIllegalAction();
		if (!bIllegal)
		{
			return FGameplayTag::EmptyTag;
		}

		FNpcFocusTarget PlayerFocus;
		PlayerFocus.FocusActor = SeenActor;
		PlayerFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Player_Hostile;
		PlayerFocus.Priority = ENpcFocusPriority::CombatTarget;
		PlayerFocus.Duration = 0.0f;

		if (!Context->RequestFocus(PlayerFocus))
		{
			return FGameplayTag::EmptyTag;
		}

		Context->AddSuspicion(100.0f);
		return StealthAiTags::TAG_NPC_State_Combat;
	}

	FNpcFocusTarget DisturbanceFocus;
	DisturbanceFocus.FocusActor = SeenActor;
	DisturbanceFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Disturbance_Environment;
	DisturbanceFocus.Priority = ENpcFocusPriority::MajorDisturbance;
	DisturbanceFocus.Duration = 5.0f;

	if (Context->RequestFocus(DisturbanceFocus))
	{
		Context->AddSuspicion(25.0f);
		return StealthAiTags::TAG_NPC_State_Alerted;
	}

	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Suspicious::HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	ENpcFocusPriority FocusPriority = ENpcFocusPriority::MinorDistraction;
	float SuspicionToAdd = 25.0f;

	switch (NoiseType)
	{
	case ENpcNoiseType::Critical:
		FocusPriority = ENpcFocusPriority::CriticalDisturbance;
		SuspicionToAdd = 75.0f;
		break;
	case ENpcNoiseType::Major:
		FocusPriority = ENpcFocusPriority::MajorDisturbance;
		SuspicionToAdd = 40.0f;
		break;
	default:
		FocusPriority = ENpcFocusPriority::MinorDistraction;
		SuspicionToAdd = 25.0f;
		break;
	}

	FNpcFocusTarget SoundFocus;
	SoundFocus.FocusLocation = Stimulus.StimulusLocation;
	SoundFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Noise_Distraction;
	SoundFocus.Priority = FocusPriority;
	SoundFocus.Duration = 4.0f;

	if (!Context->RequestFocus(SoundFocus))
	{
		return FGameplayTag::EmptyTag;
	}

	Context->AddSuspicion(SuspicionToAdd);
	return StealthAiTags::TAG_NPC_State_Alerted;
}

FGameplayTag UNpcState_Suspicious::HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	FNpcFocusTarget CrimeFocus;
	CrimeFocus.FocusLocation = Payload.CrimeLocation;
	CrimeFocus.Priority = Payload.bIsPrimaryInvestigator ? ENpcFocusPriority::CombatTarget : ENpcFocusPriority::MajorDisturbance;
	CrimeFocus.Duration = Payload.bIsPrimaryInvestigator ? 0.0f : 8.0f;

	Context->RequestFocus(CrimeFocus);
	Context->AddSuspicion(Payload.bIsPrimaryInvestigator ? 100.0f : 50.0f);
	return Payload.bIsPrimaryInvestigator ? StealthAiTags::TAG_NPC_State_Combat : StealthAiTags::TAG_NPC_State_Alerted;
}

// ---------------------------------------------------------------------------
// UNpcState_Alerted (Investigating disturbance)
// ---------------------------------------------------------------------------

UNpcState_Alerted::UNpcState_Alerted()
{
	StateTag = StealthAiTags::TAG_NPC_State_Alerted;
	AlertLevel = ENpcAlertLevel::Alerted;
	BehaviourState = ENpcBehaviourState::Alerted;
	Priority = 40;

	if (StateProfile)
	{
		StateProfile->MaxWalkSpeed = 350.0f;
		StateProfile->DefaultFocusPriority = ENpcFocusPriority::MajorDisturbance;
		StateProfile->SuspicionGainMultiplier = 1.5f;
		StateProfile->SuspicionDecayMultiplier = 0.5f;
	}
}

float UNpcState_Alerted::GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const
{
	if (Ctx.bIsFocusingOnPlayer && Ctx.bHasLineOfSight && Ctx.bIsPlayerPerformingIllegalAction)
	{
		const float BaseRate = Ctx.bIsLookingDirectlyAtPlayer ? Ctx.BaseGainRateSight : Ctx.BaseGainRatePeripheral;
		const float ProfileGainMult = StateProfile ? StateProfile->SuspicionGainMultiplier : 1.5f;
		return BaseRate * ProfileGainMult * Ctx.FocusAwarenessMultiplier * Ctx.PlayerExposureMultiplier * Ctx.DeltaTime;
	}

	const float ProfileDecayMult = StateProfile ? StateProfile->SuspicionDecayMultiplier : 0.5f;
	return -Ctx.BaseDecayRate * ProfileDecayMult * Ctx.DeltaTime;
}

FGameplayTag UNpcState_Alerted::EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const
{
	if (Ctx.bHasLineOfSight && (Ctx.bIsPlayerPerformingIllegalAction || Ctx.CurrentSuspicion >= 100.0f))
	{
		return StealthAiTags::TAG_NPC_State_Combat;
	}
	if (Ctx.CurrentSuspicion < Ctx.SuspiciousThreshold)
	{
		return StealthAiTags::TAG_NPC_State_Unaware;
	}
	if (Ctx.CurrentSuspicion < Ctx.AlertThreshold)
	{
		return StealthAiTags::TAG_NPC_State_Suspicious;
	}
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Alerted::HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	if (SeenActor == Context->GetPlayerCharacter())
	{
		const bool bIllegal = Context->IsPlayerPerformingIllegalAction();
		if (!bIllegal)
		{
			return FGameplayTag::EmptyTag;
		}

		FNpcFocusTarget PlayerFocus;
		PlayerFocus.FocusActor = SeenActor;
		PlayerFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Player_Hostile;
		PlayerFocus.Priority = ENpcFocusPriority::CombatTarget;
		PlayerFocus.Duration = 0.0f;

		if (!Context->RequestFocus(PlayerFocus))
		{
			return FGameplayTag::EmptyTag;
		}

		Context->AddSuspicion(100.0f);
		return StealthAiTags::TAG_NPC_State_Combat;
	}

	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Alerted::HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	ENpcFocusPriority FocusPriority = (NoiseType == ENpcNoiseType::Critical) ? ENpcFocusPriority::CriticalDisturbance : ENpcFocusPriority::MajorDisturbance;
	const float SuspicionToAdd = (NoiseType == ENpcNoiseType::Critical) ? 50.0f : 25.0f;

	FNpcFocusTarget SoundFocus;
	SoundFocus.FocusLocation = Stimulus.StimulusLocation;
	SoundFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Noise_Distraction;
	SoundFocus.Priority = FocusPriority;
	SoundFocus.Duration = 5.0f;

	if (Context->RequestFocus(SoundFocus))
	{
		Context->AddSuspicion(SuspicionToAdd);
	}
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Alerted::HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	FNpcFocusTarget CrimeFocus;
	CrimeFocus.FocusLocation = Payload.CrimeLocation;
	CrimeFocus.Priority = Payload.bIsPrimaryInvestigator ? ENpcFocusPriority::CombatTarget : ENpcFocusPriority::MajorDisturbance;
	CrimeFocus.Duration = Payload.bIsPrimaryInvestigator ? 0.0f : 8.0f;

	Context->RequestFocus(CrimeFocus);
	Context->AddSuspicion(Payload.bIsPrimaryInvestigator ? 100.0f : 50.0f);
	return Payload.bIsPrimaryInvestigator ? StealthAiTags::TAG_NPC_State_Combat : StealthAiTags::TAG_NPC_State_Alerted;
}

// ---------------------------------------------------------------------------
// UNpcState_Search
// ---------------------------------------------------------------------------

UNpcState_Search::UNpcState_Search()
{
	StateTag = StealthAiTags::TAG_NPC_State_Search;
	AlertLevel = ENpcAlertLevel::Alerted;
	BehaviourState = ENpcBehaviourState::Search;
	Priority = 30;

	if (StateProfile)
	{
		StateProfile->MaxWalkSpeed = 300.0f;
		StateProfile->DefaultFocusPriority = ENpcFocusPriority::MajorDisturbance;
		StateProfile->SuspicionGainMultiplier = 1.3f;
		StateProfile->SuspicionDecayMultiplier = 0.6f;
	}
}

float UNpcState_Search::GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const
{
	if (Ctx.bIsFocusingOnPlayer && Ctx.bHasLineOfSight && Ctx.bIsPlayerPerformingIllegalAction)
	{
		const float BaseRate = Ctx.bIsLookingDirectlyAtPlayer ? Ctx.BaseGainRateSight : Ctx.BaseGainRatePeripheral;
		const float ProfileGainMult = StateProfile ? StateProfile->SuspicionGainMultiplier : 1.3f;
		return BaseRate * ProfileGainMult * Ctx.FocusAwarenessMultiplier * Ctx.PlayerExposureMultiplier * Ctx.DeltaTime;
	}

	const float ProfileDecayMult = StateProfile ? StateProfile->SuspicionDecayMultiplier : 0.6f;
	return -Ctx.BaseDecayRate * ProfileDecayMult * Ctx.DeltaTime;
}

FGameplayTag UNpcState_Search::EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const
{
	if (Ctx.bHasLineOfSight && (Ctx.bIsPlayerPerformingIllegalAction || Ctx.CurrentSuspicion >= Ctx.AlertThreshold))
	{
		return StealthAiTags::TAG_NPC_State_Combat;
	}
	if (Ctx.SearchDuration >= 10.0f || Ctx.CurrentSuspicion < Ctx.SuspiciousThreshold)
	{
		return StealthAiTags::TAG_NPC_State_Unaware;
	}
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Search::HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	if (SeenActor == Context->GetPlayerCharacter())
	{
		const bool bIllegal = Context->IsPlayerPerformingIllegalAction();
		if (!bIllegal)
		{
			return FGameplayTag::EmptyTag;
		}

		FNpcFocusTarget PlayerFocus;
		PlayerFocus.FocusActor = SeenActor;
		PlayerFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Player_Hostile;
		PlayerFocus.Priority = ENpcFocusPriority::CombatTarget;
		PlayerFocus.Duration = 0.0f;

		if (!Context->RequestFocus(PlayerFocus))
		{
			return FGameplayTag::EmptyTag;
		}

		Context->AddSuspicion(100.0f);
		return StealthAiTags::TAG_NPC_State_Combat;
	}
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Search::HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	// Refreshes search focus location without resetting or demoting search progress
	ENpcFocusPriority FocusPriority = (NoiseType == ENpcNoiseType::Critical) ? ENpcFocusPriority::CriticalDisturbance : ENpcFocusPriority::MajorDisturbance;
	FNpcFocusTarget SoundFocus;
	SoundFocus.FocusLocation = Stimulus.StimulusLocation;
	SoundFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Noise_Distraction;
	SoundFocus.Priority = FocusPriority;
	SoundFocus.Duration = 5.0f;

	if (Context->RequestFocus(SoundFocus))
	{
		Context->AddSuspicion(20.0f);
	}

	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Search::HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	FNpcFocusTarget CrimeFocus;
	CrimeFocus.FocusLocation = Payload.CrimeLocation;
	CrimeFocus.Priority = Payload.bIsPrimaryInvestigator ? ENpcFocusPriority::CombatTarget : ENpcFocusPriority::MajorDisturbance;
	CrimeFocus.Duration = Payload.bIsPrimaryInvestigator ? 0.0f : 8.0f;

	Context->RequestFocus(CrimeFocus);
	Context->AddSuspicion(Payload.bIsPrimaryInvestigator ? 100.0f : 50.0f);
	return Payload.bIsPrimaryInvestigator ? StealthAiTags::TAG_NPC_State_Combat : StealthAiTags::TAG_NPC_State_Alerted;
}

// ---------------------------------------------------------------------------
// UNpcState_Combat
// ---------------------------------------------------------------------------

UNpcState_Combat::UNpcState_Combat()
{
	StateTag = StealthAiTags::TAG_NPC_State_Combat;
	AlertLevel = ENpcAlertLevel::Hostile;
	BehaviourState = ENpcBehaviourState::Combat;
	Priority = 100;

	if (StateProfile)
	{
		StateProfile->MaxWalkSpeed = 500.0f;
		StateProfile->DefaultFocusPriority = ENpcFocusPriority::CombatTarget;
		StateProfile->SuspicionGainMultiplier = 2.0f;
		StateProfile->SuspicionDecayMultiplier = 0.0f;
	}
}

float UNpcState_Combat::GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const
{
	// Suspicion frozen while player is in sight during combat; decays when sight is lost
	return Ctx.bHasLineOfSight ? 0.0f : -Ctx.BaseDecayRate * Ctx.DeltaTime;
}

FGameplayTag UNpcState_Combat::EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const
{
	const bool bStillHostile = Ctx.bEffectivelySeesPlayer || Ctx.bIsPlayerPerformingIllegalAction || Ctx.CurrentSuspicion >= Ctx.AlertThreshold;
	if (Ctx.bHasLineOfSight && bStillHostile)
	{
		return FGameplayTag::EmptyTag; // Stay in combat
	}

	if (Ctx.LostSightDuration >= Ctx.SightLossGrace || Ctx.CurrentSuspicion < Ctx.AlertThreshold)
	{
		return StealthAiTags::TAG_NPC_State_Search;
	}

	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Combat::HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus)
{
	if (!Context)
	{
		return FGameplayTag::EmptyTag;
	}

	if (SeenActor == Context->GetPlayerCharacter())
	{
		FNpcFocusTarget PlayerFocus;
		PlayerFocus.FocusActor = SeenActor;
		PlayerFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Player_Hostile;
		PlayerFocus.Priority = ENpcFocusPriority::CombatTarget;
		PlayerFocus.Duration = 0.0f; // Indefinite combat focus

		Context->RequestFocus(PlayerFocus);
	}

	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Combat::HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType)
{
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Combat::HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload)
{
	return FGameplayTag::EmptyTag;
}

// ---------------------------------------------------------------------------
// UNpcState_Fleeing (Open/Closed Extensibility Demo State)
// ---------------------------------------------------------------------------

UNpcState_Fleeing::UNpcState_Fleeing()
{
	StateTag = StealthAiTags::TAG_NPC_State_Fleeing;
	AlertLevel = ENpcAlertLevel::Hostile;
	BehaviourState = ENpcBehaviourState::Alerted;
	Priority = 90;

	if (StateProfile)
	{
		StateProfile->MaxWalkSpeed = 450.0f;
		StateProfile->DefaultFocusPriority = ENpcFocusPriority::MajorDisturbance;
		StateProfile->SuspicionGainMultiplier = 1.0f;
		StateProfile->SuspicionDecayMultiplier = 1.0f;
	}
}

float UNpcState_Fleeing::GetSuspicionDelta_Implementation(const FNpcStateTickContext& Ctx) const
{
	return -Ctx.BaseDecayRate * Ctx.DeltaTime;
}

FGameplayTag UNpcState_Fleeing::EvaluateTransition_Implementation(const FNpcStateTickContext& Ctx) const
{
	if (Ctx.LostSightDuration >= 8.0f && Ctx.CurrentSuspicion < Ctx.SuspiciousThreshold)
	{
		return StealthAiTags::TAG_NPC_State_Unaware;
	}
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Fleeing::HandleSightStimulus_Implementation(UNpcContextComponent* Context, AActor* SeenActor, const FAIStimulus& Stimulus)
{
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Fleeing::HandleHearingStimulus_Implementation(UNpcContextComponent* Context, AActor* StimulusSourceActor, const FAIStimulus& Stimulus, ENpcNoiseType NoiseType)
{
	return FGameplayTag::EmptyTag;
}

FGameplayTag UNpcState_Fleeing::HandleCrimeReported_Implementation(UNpcContextComponent* Context, const FAiCrimeEventPayload& Payload)
{
	return FGameplayTag::EmptyTag;
}
