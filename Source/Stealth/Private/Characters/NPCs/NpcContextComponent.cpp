#include "Characters/NPCs/NpcContextComponent.h"
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

	CurrentStateTag = StealthAiTags::TAG_NPC_State_Unaware;

	if (const UCharactersRegistrySubsystem* Registry = UCharactersRegistrySubsystem::Get(this))
	{
		if (NpcAiController)
		{
			OwnerNpcGuid = Registry->GetNpcGuidByController(NpcAiController);
		}
		else if (ANpcCharacter* NpcChar = Cast<ANpcCharacter>(GetOwner()))
		{
			OwnerNpcGuid = Registry->GetNpcGuidByCharacter(NpcChar);
		}
	}

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

bool UNpcContextComponent::IsPlayerPerformingIllegalAction(const AStealthPlayerCharacter* Player) const
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

void UNpcContextComponent::UpdateSuspicion(float DeltaTime)
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

void UNpcContextComponent::EvaluateAlertState()
{
	const float AlertThreshold = Profile ? Profile->SuspicionThreshold_Alert : 100.0f;

	FGameplayTag TargetStateTag = StealthAiTags::TAG_NPC_State_Unaware;

	if (CurrentSuspicion >= 100.0f || (bIsPlayerInRestrictedArea && bEffectivelySeesPlayer))
	{
		TargetStateTag = StealthAiTags::TAG_NPC_State_Combat;
	}
	else if (CurrentSuspicion >= AlertThreshold)
	{
		TargetStateTag = StealthAiTags::TAG_NPC_State_Alerted;
	}
	else if (CurrentSuspicion > 5.0f)
	{
		TargetStateTag = StealthAiTags::TAG_NPC_State_Suspicious;
	}
	else
	{
		TargetStateTag = StealthAiTags::TAG_NPC_State_Unaware;
	}

	SetNpcState(TargetStateTag);
}

void UNpcContextComponent::SetNpcState(const FGameplayTag& NewStateTag)
{
	if (CurrentStateTag == NewStateTag || !NewStateTag.IsValid())
	{
		return;
	}

	const FGameplayTag PreviousStateTag = CurrentStateTag;
	CurrentStateTag = NewStateTag;

	// Synchronize legacy enum properties for backwards compatibility
	if (NewStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Combat) || NewStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Dead))
	{
		AlertLevel = ENpcAlertLevel::Hostile;
		CurrentBehaviourState = EGuardBehaviourState::Alarm;
	}
	else if (NewStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Alerted) || NewStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Search))
	{
		AlertLevel = ENpcAlertLevel::Alerted;
		CurrentBehaviourState = (NewStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Search)) ? EGuardBehaviourState::Search : EGuardBehaviourState::Alerted;
	}
	else if (NewStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Suspicious))
	{
		AlertLevel = ENpcAlertLevel::Suspicious;
		CurrentBehaviourState = EGuardBehaviourState::Suspicious;
	}
	else
	{
		AlertLevel = ENpcAlertLevel::Unaware;
		CurrentBehaviourState = EGuardBehaviourState::Patrol;
	}

	// 1. Notify StateTree directly so behavior transitions immediately
	SendStateTreeEvent(CurrentStateTag);

	// 2. Broadcast internal delegates for local listeners (UI, AnimInstance, Character)
	OnNpcStateChanged.Broadcast(CurrentStateTag, PreviousStateTag);
	OnAlertLevelChanged.Broadcast(AlertLevel);
	OnBehaviourStateChanged.Broadcast(CurrentBehaviourState);

	// 3. Broadcast global message via GameplayMessageSubsystem for loose coupling across systems
	if (UWorld* World = GetWorld())
	{
		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(World);
		FNpcStateChangedMessage Msg;
		Msg.NpcGUID = OwnerNpcGuid;
		Msg.PreviousStateTag = PreviousStateTag;
		Msg.NewStateTag = CurrentStateTag;
		Msg.Suspicion = CurrentSuspicion;
		MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_NPC_StateChanged, Msg);
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

	SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_NoiseHeard);
}

void UNpcContextComponent::HandleCrimeReported(const FAiCrimeEventPayload& CrimePayload)
{
	LastHeardSoundLocation = CrimePayload.CrimeLocation;
	LastKnownPlayerPos = CrimePayload.CrimeLocation;
	TimeSinceLastStimulus = 0.0f;

	if (CrimePayload.bIsPrimaryInvestigator)
	{
		AddSuspicion(100.0f);
		SetNpcState(StealthAiTags::TAG_NPC_State_Combat);
	}
	else
	{
		AddSuspicion(50.0f);
		SetNpcState(StealthAiTags::TAG_NPC_State_Alerted);
	}

	SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_CrimeReported);
}

void UNpcContextComponent::AddSuspicion(float Amount)
{
	CurrentSuspicion = FMath::Clamp(CurrentSuspicion + Amount, 0.0f, 100.0f);
	OnSuspicionChanged.Broadcast(CurrentSuspicion);
	EvaluateAlertState();
}

void UNpcContextComponent::SetAlertLevel(ENpcAlertLevel NewAlertLevel)
{
	switch (NewAlertLevel)
	{
	case ENpcAlertLevel::Hostile:
		SetNpcState(StealthAiTags::TAG_NPC_State_Combat);
		break;
	case ENpcAlertLevel::Alerted:
		SetNpcState(StealthAiTags::TAG_NPC_State_Alerted);
		break;
	case ENpcAlertLevel::Suspicious:
		SetNpcState(StealthAiTags::TAG_NPC_State_Suspicious);
		break;
	case ENpcAlertLevel::Unaware:
	default:
		SetNpcState(StealthAiTags::TAG_NPC_State_Unaware);
		break;
	}
}

void UNpcContextComponent::SetBehaviourState(EGuardBehaviourState NewState)
{
	switch (NewState)
	{
	case EGuardBehaviourState::Alarm:
		SetNpcState(StealthAiTags::TAG_NPC_State_Combat);
		break;
	case EGuardBehaviourState::Search:
		SetNpcState(StealthAiTags::TAG_NPC_State_Search);
		break;
	case EGuardBehaviourState::Alerted:
		SetNpcState(StealthAiTags::TAG_NPC_State_Alerted);
		break;
	case EGuardBehaviourState::Suspicious:
		SetNpcState(StealthAiTags::TAG_NPC_State_Suspicious);
		break;
	case EGuardBehaviourState::Patrol:
	default:
		SetNpcState(StealthAiTags::TAG_NPC_State_Unaware);
		break;
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
	}
}
