#include "Characters/NPCs/AI/Suspicion/NpcPerceptionComponent.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/AI/States/NpcState.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Engine/World.h"
#include "Exposure/PlayerExposureSubsystem.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Legality/LegalitySubsystem.h"
#include "Messages/StealthMessages.h"
#include "Perception/AIPerceptionTypes.h"

UNpcPerceptionComponent::UNpcPerceptionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.05f; // 20 Hz ceiling
}

void UNpcPerceptionComponent::BeginPlay()
{
	Super::BeginPlay();

	GetAiController();
	GetFocusComponent();
	GetContextComponent();

	InitializeDefaultStates();
}

void UNpcPerceptionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PlayerInRestrictedAreaListenerHandle.IsValid())
	{
		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
		MsgSubsystem.UnregisterListener(PlayerInRestrictedAreaListenerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UNpcPerceptionComponent::InitializeDefaultStates()
{
	StateRegistry.Empty();

	if (States.Num() == 0)
	{
		States.Add(NewObject<UNpcState_Unaware>(this, UNpcState_Unaware::StaticClass()));
		States.Add(NewObject<UNpcState_Suspicious>(this, UNpcState_Suspicious::StaticClass()));
		States.Add(NewObject<UNpcState_Alerted>(this, UNpcState_Alerted::StaticClass()));
		States.Add(NewObject<UNpcState_Search>(this, UNpcState_Search::StaticClass()));
		States.Add(NewObject<UNpcState_Combat>(this, UNpcState_Combat::StaticClass()));
		States.Add(NewObject<UNpcState_Fleeing>(this, UNpcState_Fleeing::StaticClass()));
	}

	for (UNpcState* State : States)
	{
		if (State)
		{
			StateRegistry.Add(State->GetStateTag(), State);
		}
	}

	if (!CurrentState)
	{
		TransitionToStateByTag(StealthAiTags::TAG_NPC_State_Unaware);
	}
}

void UNpcPerceptionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdatePerceptionState(DeltaTime);

	const FNpcStateTickContext Ctx = BuildTickContext(DeltaTime);

	if (CurrentState)
	{
		// 1. Calculate and apply suspicion delta from active state
		const float Delta = CurrentState->GetSuspicionDelta(Ctx);
		const float PreviousSuspicion = CurrentSuspicion;
		CurrentSuspicion = FMath::Clamp(CurrentSuspicion + Delta, 0.0f, 100.0f);

		if (!FMath::IsNearlyEqual(CurrentSuspicion, PreviousSuspicion, 0.01f))
		{
			OnSuspicionChanged.Broadcast(CurrentSuspicion);
		}

		// 2. Tick current state
		CurrentState->OnTickState(GetContextComponent(), DeltaTime);

		// 3. Evaluate transitions from active state
		FNpcStateTickContext TransitionCtx = Ctx;
		TransitionCtx.CurrentSuspicion = CurrentSuspicion;

		const FGameplayTag NextStateTag = CurrentState->EvaluateTransition(TransitionCtx);
		if (NextStateTag.IsValid() && NextStateTag != CurrentStateTag)
		{
			TransitionToStateByTag(NextStateTag);
		}
	}
}

ANpcAiController* UNpcPerceptionComponent::GetAiController() const
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

UNpcFocusComponent* UNpcPerceptionComponent::GetFocusComponent() const
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

UNpcContextComponent* UNpcPerceptionComponent::GetContextComponent() const
{
	if (!CachedContextComponent.IsValid())
	{
		if (GetOwner())
		{
			CachedContextComponent = GetOwner()->FindComponentByClass<UNpcContextComponent>();
		}
		if (!CachedContextComponent.IsValid())
		{
			if (const ANpcAiController* Controller = GetAiController())
			{
				CachedContextComponent = Controller->FindComponentByClass<UNpcContextComponent>();
			}
		}
	}
	return CachedContextComponent.Get();
}

AStealthPlayerCharacter* UNpcPerceptionComponent::GetPlayerCharacter() const
{
	if (!CachedPlayerCharacter.IsValid())
	{
		if (const UCharactersRegistrySubsystem* CharactersRegistry = UCharactersRegistrySubsystem::Get(this))
		{
			CachedPlayerCharacter = CharactersRegistry->GetPlayerCharacter();
		}
	}
	return CachedPlayerCharacter.Get();
}

bool UNpcPerceptionComponent::IsPlayerPerformingIllegalAction() const
{
	if (const ULegalitySubsystem* Legality = ULegalitySubsystem::Get(GetWorld()))
	{
		return Legality->IsPlayerPerformingIllegalAction();
	}
	return false;
}

UNpcState* UNpcPerceptionComponent::GetStateByTag(const FGameplayTag& StateTag) const
{
	if (const TObjectPtr<UNpcState>* Found = StateRegistry.Find(StateTag))
	{
		return Found->Get();
	}
	return nullptr;
}

void UNpcPerceptionComponent::RegisterState(UNpcState* NewState)
{
	if (NewState)
	{
		StateRegistry.Add(NewState->GetStateTag(), NewState);
		if (!States.Contains(NewState))
		{
			States.Add(NewState);
		}
	}
}

void UNpcPerceptionComponent::UnregisterState(UNpcState* StateToRemove)
{
	if (StateToRemove)
	{
		StateRegistry.Remove(StateToRemove->GetStateTag());
		States.Remove(StateToRemove);
	}
}

bool UNpcPerceptionComponent::TransitionToState(UNpcState* NewState)
{
	if (!NewState || CurrentState == NewState)
	{
		return false;
	}

	UNpcContextComponent* Context = GetContextComponent();

	// Guard: check if transition is allowed
	if (CurrentState && !CurrentState->CanTransitionTo(Context, NewState))
	{
		return false;
	}

	UNpcState* PreviousState = CurrentState;
	const FGameplayTag PreviousStateTag = CurrentStateTag;

	if (PreviousState)
	{
		PreviousState->OnExitState(Context, NewState);
	}

	// Update single-source-of-truth state tag & pointer
	CurrentStateTag = NewState->GetStateTag();
	CurrentState = NewState;

	// Resolve and apply StateProfile exactly once (per-NPC profile override takes precedence over state default)
	UNpcStateProfile* ProfileToApply = Profile ? Profile->GetStateProfile(CurrentStateTag) : nullptr;
	if (!ProfileToApply)
	{
		ProfileToApply = NewState->GetStateProfile();
	}

	if (ProfileToApply && Context)
	{
		ProfileToApply->ApplyToNpc(Context);
	}

	CurrentState->OnEnterState(Context, PreviousState);

	const ENpcAlertLevel OldAlertLevel = PreviousState ? PreviousState->GetAlertLevel() : ENpcAlertLevel::Unaware;
	const ENpcBehaviourState OldBehaviour = PreviousState ? PreviousState->GetBehaviourState() : ENpcBehaviourState::Routine;
	const ENpcAlertLevel NewAlertLevel = NewState->GetAlertLevel();
	const ENpcBehaviourState NewBehaviour = NewState->GetBehaviourState();

	// Send StateTree events if transitioning
	if (Context)
	{
		Context->SendStateTreeEvent(CurrentStateTag);

		if (PreviousStateTag == StealthAiTags::TAG_NPC_State_Search && CurrentStateTag == StealthAiTags::TAG_NPC_State_Unaware)
		{
			Context->SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_SearchExpired);
			Context->SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_ResumeRoutine);
		}
	}

	// Broadcast changes
	OnNpcStateChanged.Broadcast(CurrentStateTag, PreviousStateTag);

	if (NewAlertLevel != OldAlertLevel)
	{
		OnAlertLevelChanged.Broadcast(NewAlertLevel);
	}

	if (NewBehaviour != OldBehaviour)
	{
		OnBehaviourStateChanged.Broadcast(NewBehaviour);
	}

	OnAlertStateEvaluated.Broadcast(CurrentStateTag, NewAlertLevel);

	return true;
}

bool UNpcPerceptionComponent::TransitionToStateByTag(const FGameplayTag& StateTag)
{
	if (UNpcState* TargetState = GetStateByTag(StateTag))
	{
		return TransitionToState(TargetState);
	}
	return false;
}

void UNpcPerceptionComponent::UpdatePerceptionState(float DeltaTime)
{
	const AStealthPlayerCharacter* Player = GetPlayerCharacter();
	const bool bHasPlayer = (Player != nullptr);
	const bool bIsIllegal = bHasPlayer && IsPlayerPerformingIllegalAction();

	if (bHasPlayer && bHasPlayerLineOfSight)
	{
		LostPlayerSightDuration = 0.0f;
	}
	else
	{
		LostPlayerSightDuration += DeltaTime;
		if (GetBehaviourState() == ENpcBehaviourState::Search)
		{
			SearchDurationTimer += DeltaTime;
		}
	}

	const bool bWasEffectivelySeeing = bEffectivelySeesPlayer;
	const bool bIsHostile = (GetAlertLevel() == ENpcAlertLevel::Hostile);
	bEffectivelySeesPlayer = bHasPlayerLineOfSight && (bIsIllegal || (bStimulusIsFromPlayer && CurrentSuspicion > 50.0f) || bIsHostile);

	if (bWasEffectivelySeeing != bEffectivelySeesPlayer)
	{
		OnPlayerInSightChanged.Broadcast(bEffectivelySeesPlayer);
	}
}

FNpcStateTickContext UNpcPerceptionComponent::BuildTickContext(float DeltaTime) const
{
	FNpcStateTickContext Ctx;
	Ctx.DeltaTime = DeltaTime;
	Ctx.CurrentSuspicion = CurrentSuspicion;
	Ctx.bHasLineOfSight = bHasPlayerLineOfSight;
	Ctx.bEffectivelySeesPlayer = bEffectivelySeesPlayer;
	Ctx.bIsPlayerPerformingIllegalAction = IsPlayerPerformingIllegalAction();

	const AStealthPlayerCharacter* Player = GetPlayerCharacter();
	if (const UNpcFocusComponent* FocusComp = GetFocusComponent())
	{
		const FNpcFocusTarget& Focus = FocusComp->GetCurrentFocus();
		Ctx.bIsFocusingOnPlayer = (Player != nullptr && Focus.FocusActor.Get() == Player);
		Ctx.FocusAwarenessMultiplier = FocusComp->GetAwarenessMultiplier();
	}
	else
	{
		Ctx.bIsFocusingOnPlayer = false;
		Ctx.FocusAwarenessMultiplier = 1.0f;
	}

	if (Player && GetOwner())
	{
		Ctx.PlayerDistance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
		Ctx.bIsLookingDirectlyAtPlayer = IsLookingDirectlyAtPlayer(Player);
	}

	Ctx.PlayerExposureMultiplier = CalculatePlayerExposureMultiplier();
	Ctx.LostSightDuration = LostPlayerSightDuration;
	Ctx.SearchDuration = SearchDurationTimer;

	Ctx.BaseGainRateSight = Profile ? Profile->SuspicionGainPerSecond_Sight : 40.0f;
	Ctx.BaseGainRatePeripheral = Profile ? Profile->SuspicionGainPerSecond_Peripheral : 15.0f;
	Ctx.BaseDecayRate = Profile ? Profile->SuspicionDecayPerSecond : 8.0f;
	Ctx.AlertThreshold = Profile ? Profile->SuspicionThreshold_Alert : 75.0f;
	Ctx.SuspiciousThreshold = 25.0f;
	Ctx.SightLossGrace = Profile ? Profile->LosePlayerSightGracePeriod : 2.0f;
	Ctx.AlwaysSeeRange = Profile ? Profile->AlwaysSeePlayerRange : 100.0f;

	return Ctx;
}

void UNpcPerceptionComponent::OnSightStimulus(const AActor* TargetActor, const FAIStimulus& Stimulus)
{
	if (!TargetActor || TargetActor != GetPlayerCharacter())
	{
		//TODO: add support for non-player actors
		return;
	}

	FVector SeenActorLocation = TargetActor->GetActorLocation();
	float DistanceFromActor = FVector::Distance(SeenActorLocation, CachedController->GetPawn()->GetActorLocation());

	if (DistanceFromActor <= CurrentState->GetStateProfile()->AlwaysSeePlayerRange)
	{
		bHasPlayerLineOfSight = true;
	}
	else
	{
		bHasPlayerLineOfSight = (Stimulus.WasSuccessfullySensed() && Stimulus.Strength * UPlayerExposureSubsystem::Get(this)->GetCurrentTotalExposure() >= 0.05f);
	}

	if (bHasPlayerLineOfSight)
	{
		LastKnownPlayerPos = SeenActorLocation;
	}
	else
	{
		return;
	}

	// Route stimulus to active UNpcState instance
	UNpcContextComponent* Context = GetContextComponent();
	if (CurrentState)
	{
		const FGameplayTag TransitionTag = CurrentState->HandleSightStimulus(Context, const_cast<AActor*>(TargetActor), Stimulus);
		if (TransitionTag.IsValid() && TransitionTag != CurrentStateTag)
		{
			TransitionToStateByTag(TransitionTag);
		}
	}
}

ENpcNoiseType UNpcPerceptionComponent::ClassifyNoiseStimulus(const FAIStimulus& Stimulus) const
{
	if (Stimulus.Tag == StealthAiTags::TAG_Noise_Critical.GetTag().GetTagName())
	{
		return ENpcNoiseType::Critical;
	}
	if (Stimulus.Tag == StealthAiTags::TAG_Noise_Major.GetTag().GetTagName())
	{
		return ENpcNoiseType::Major;
	}
	if (Stimulus.Tag == StealthAiTags::TAG_Noise_Distraction.GetTag().GetTagName())
	{
		return ENpcNoiseType::Distraction;
	}
	if (Stimulus.Tag == StealthAiTags::TAG_Noise_Footstep.GetTag().GetTagName())
	{
		return ENpcNoiseType::Subtle;
	}

	if (Stimulus.Strength >= 0.85f)
	{
		return ENpcNoiseType::Critical;
	}
	if (Stimulus.Strength >= 0.6f)
	{
		return ENpcNoiseType::Major;
	}
	if (Stimulus.Strength >= 0.3f)
	{
		return ENpcNoiseType::Distraction;
	}

	return ENpcNoiseType::Subtle;
}

bool UNpcPerceptionComponent::ShouldReactToNoise(ENpcNoiseType NoiseType, const FAIStimulus& Stimulus) const
{
	if (GetBehaviourState() == ENpcBehaviourState::Combat || GetAlertLevel() == ENpcAlertLevel::Hostile)
	{
		return false;
	}

	if (NoiseType == ENpcNoiseType::Subtle)
	{
		return false;
	}

	return true;
}

bool UNpcPerceptionComponent::OnHearingStimulus(AActor* TargetActor, const FAIStimulus& Stimulus)
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

	const bool bIsPlayer = (TargetActor != nullptr && TargetActor == GetPlayerCharacter());
	bStimulusIsFromPlayer = bIsPlayer;
	CurrentStimulusSourceActor = TargetActor;
	LastHeardSoundLocation = Stimulus.StimulusLocation;

	UNpcContextComponent* Context = GetContextComponent();
	if (CurrentState)
	{
		const FGameplayTag TransitionTag = CurrentState->HandleHearingStimulus(Context, TargetActor, Stimulus, NoiseType);
		if (TransitionTag.IsValid() && TransitionTag != CurrentStateTag)
		{
			TransitionToStateByTag(TransitionTag);
		}
	}

	if (Context)
	{
		Context->SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_NoiseHeard);
	}

	OnNoiseHeard.Broadcast(NoiseType, Stimulus.StimulusLocation);
	return true;
}

void UNpcPerceptionComponent::OnPredictionStimulus(AActor* TargetActor, const FAIStimulus& Stimulus) {}
void UNpcPerceptionComponent::OnTargetPerceptionForgotten(AActor* TargetActor) {}

void UNpcPerceptionComponent::HandleCrimeReported(const FAiCrimeEventPayload& CrimePayload)
{
	LastHeardSoundLocation = CrimePayload.CrimeLocation;
	LastKnownPlayerPos = CrimePayload.CrimeLocation;
	bStimulusIsFromPlayer = true;

	UNpcContextComponent* Context = GetContextComponent();
	if (CurrentState)
	{
		const FGameplayTag TransitionTag = CurrentState->HandleCrimeReported(Context, CrimePayload);
		if (TransitionTag.IsValid())
		{
			TransitionToStateByTag(TransitionTag);
			return;
		}
	}

	// Fallback crime handling
	if (CrimePayload.bIsPrimaryInvestigator)
	{
		AddSuspicion(100.0f);
		TransitionToStateByTag(StealthAiTags::TAG_NPC_State_Combat);
	}
	else
	{
		AddSuspicion(50.0f);
		TransitionToStateByTag(StealthAiTags::TAG_NPC_State_Alerted);
	}
}

void UNpcPerceptionComponent::AddSuspicion(float Amount)
{
	const float PrevSuspicion = CurrentSuspicion;
	CurrentSuspicion = FMath::Clamp(CurrentSuspicion + Amount, 0.0f, 100.0f);

	if (!FMath::IsNearlyEqual(CurrentSuspicion, PrevSuspicion, 0.01f))
	{
		OnSuspicionChanged.Broadcast(CurrentSuspicion);
	}
}

void UNpcPerceptionComponent::SetSuspicion(float Value)
{
	const float PrevSuspicion = CurrentSuspicion;
	CurrentSuspicion = FMath::Clamp(Value, 0.0f, 100.0f);

	if (!FMath::IsNearlyEqual(CurrentSuspicion, PrevSuspicion, 0.01f))
	{
		OnSuspicionChanged.Broadcast(CurrentSuspicion);
	}
}

void UNpcPerceptionComponent::SetAlertLevel(ENpcAlertLevel NewAlertLevel)
{
	FGameplayTag TargetTag = StealthAiTags::TAG_NPC_State_Unaware;
	switch (NewAlertLevel)
	{
	case ENpcAlertLevel::Hostile:
		TargetTag = StealthAiTags::TAG_NPC_State_Combat;
		break;
	case ENpcAlertLevel::Alerted:
		TargetTag = StealthAiTags::TAG_NPC_State_Alerted;
		break;
	case ENpcAlertLevel::Suspicious:
		TargetTag = StealthAiTags::TAG_NPC_State_Suspicious;
		break;
	case ENpcAlertLevel::Unaware:
	default:
		TargetTag = StealthAiTags::TAG_NPC_State_Unaware;
		break;
	}

	TransitionToStateByTag(TargetTag);
}

void UNpcPerceptionComponent::SetBehaviourState(ENpcBehaviourState NewState)
{
	FGameplayTag TargetTag = StealthAiTags::TAG_NPC_State_Unaware;
	switch (NewState)
	{
	case ENpcBehaviourState::Combat:
		TargetTag = StealthAiTags::TAG_NPC_State_Combat;
		break;
	case ENpcBehaviourState::Search:
		TargetTag = StealthAiTags::TAG_NPC_State_Search;
		break;
	case ENpcBehaviourState::Alerted:
		TargetTag = StealthAiTags::TAG_NPC_State_Alerted;
		break;
	case ENpcBehaviourState::Suspicious:
		TargetTag = StealthAiTags::TAG_NPC_State_Suspicious;
		break;
	case ENpcBehaviourState::Routine:
	default:
		TargetTag = StealthAiTags::TAG_NPC_State_Unaware;
		break;
	}

	TransitionToStateByTag(TargetTag);
}

ENpcAlertLevel UNpcPerceptionComponent::GetAlertLevel() const
{
	return CurrentState ? CurrentState->GetAlertLevel() : ENpcAlertLevel::Unaware;
}

ENpcBehaviourState UNpcPerceptionComponent::GetBehaviourState() const
{
	return CurrentState ? CurrentState->GetBehaviourState() : ENpcBehaviourState::Routine;
}

float UNpcPerceptionComponent::CalculatePlayerExposureMultiplier() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const UPlayerExposureSubsystem* ExposureSubsystem = World->GetSubsystem<UPlayerExposureSubsystem>())
		{
			return ExposureSubsystem->GetCurrentTotalExposure();
		}
	}
	return 1.0f;
}

bool UNpcPerceptionComponent::IsLookingDirectlyAtPlayer(const AStealthPlayerCharacter* Player) const
{
	if (!Player)
	{
		return false;
	}

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return false;
	}

	const FVector NpcForward = OwnerActor->GetActorForwardVector();
	const FVector ToPlayer = (Player->GetActorLocation() - OwnerActor->GetActorLocation()).GetSafeNormal();
	const float Dot = FVector::DotProduct(NpcForward, ToPlayer);

	// In front arc (> 45 degrees)
	return Dot >= 0.707f;
}
