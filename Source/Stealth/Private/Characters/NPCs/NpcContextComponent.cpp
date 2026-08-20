#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcCharacter.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/AI/Suspicion/NpcSuspicionComponent.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/StealthMessages.h"
#include "Stealth/Stealth.h"

namespace
{
	static const FVector StaticZeroVector = FVector::ZeroVector;
	static const FNpcFocusTarget StaticEmptyFocusTarget;
}

UNpcContextComponent::UNpcContextComponent()
{
	PrimaryComponentTick.bCanEverTick = false; // Context is an interface facade; ticking is handled by dedicated components
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

	BindToSubcomponents();
}

void UNpcContextComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SuspicionComponent)
	{
		SuspicionComponent->OnSuspicionChanged.RemoveDynamic(this, &UNpcContextComponent::HandleSuspicionChanged);
		SuspicionComponent->OnAlertStateEvaluated.RemoveDynamic(this, &UNpcContextComponent::HandleAlertStateEvaluated);
		SuspicionComponent->OnPlayerInSightChanged.RemoveDynamic(this, &UNpcContextComponent::HandlePlayerInSightChanged);
		SuspicionComponent->OnNoiseHeard.RemoveDynamic(this, &UNpcContextComponent::HandleNoiseHeard);
	}

	if (FocusComponent)
	{
		FocusComponent->OnNpcFocusChanged.RemoveDynamic(this, &UNpcContextComponent::HandleFocusChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void UNpcContextComponent::BindToSubcomponents()
{
	UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent();
	if (SuspicionComp)
	{
		if (Profile && !SuspicionComp->GetProfile())
		{
			SuspicionComp->SetProfile(Profile);
		}

		SuspicionComp->OnSuspicionChanged.AddUniqueDynamic(this, &UNpcContextComponent::HandleSuspicionChanged);
		SuspicionComp->OnAlertStateEvaluated.AddUniqueDynamic(this, &UNpcContextComponent::HandleAlertStateEvaluated);
		SuspicionComp->OnPlayerInSightChanged.AddUniqueDynamic(this, &UNpcContextComponent::HandlePlayerInSightChanged);
		SuspicionComp->OnNoiseHeard.AddUniqueDynamic(this, &UNpcContextComponent::HandleNoiseHeard);
	}

	UNpcFocusComponent* FocusComp = GetFocusComponent();
	if (FocusComp)
	{
		FocusComp->OnNpcFocusChanged.AddUniqueDynamic(this, &UNpcContextComponent::HandleFocusChanged);
	}
}

UNpcSuspicionComponent* UNpcContextComponent::GetSuspicionComponent() const
{
	if (!SuspicionComponent)
	{
		if (GetOwner())
		{
			SuspicionComponent = GetOwner()->FindComponentByClass<UNpcSuspicionComponent>();
		}
		if (!SuspicionComponent && NpcAiController)
		{
			SuspicionComponent = NpcAiController->FindComponentByClass<UNpcSuspicionComponent>();
		}
	}
	return SuspicionComponent;
}

UNpcFocusComponent* UNpcContextComponent::GetFocusComponent() const
{
	if (!FocusComponent)
	{
		if (GetOwner())
		{
			FocusComponent = GetOwner()->FindComponentByClass<UNpcFocusComponent>();
		}
		if (!FocusComponent && NpcAiController)
		{
			FocusComponent = NpcAiController->FindComponentByClass<UNpcFocusComponent>();
		}
	}
	return FocusComponent;
}

void UNpcContextComponent::SetProfile(UNpcProfile* InProfile)
{
	Profile = InProfile;
	if (UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		SuspicionComp->SetProfile(InProfile);
	}
}

bool UNpcContextComponent::IsPlayerInRestrictedArea() const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->IsPlayerInRestrictedArea();
	}
	return false;
}

bool UNpcContextComponent::IsPlayerPerformingIllegalAction(const AStealthPlayerCharacter* Player) const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->IsPlayerPerformingIllegalAction(Player);
	}
	return false;
}

float UNpcContextComponent::GetAwareness() const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->GetSuspicion();
	}
	return 0.0f;
}

ENpcAlertLevel UNpcContextComponent::GetAlertLevel() const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->GetAlertLevel();
	}
	return ENpcAlertLevel::Unaware;
}

ENpcBehaviourState UNpcContextComponent::GetBehaviourState() const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->GetBehaviourState();
	}
	return ENpcBehaviourState::Routine;
}

bool UNpcContextComponent::HasPlayerLineOfSight() const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->HasPlayerLineOfSight();
	}
	return false;
}

bool UNpcContextComponent::EffectivelySeesPlayer() const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->EffectivelySeesPlayer();
	}
	return false;
}

const FVector& UNpcContextComponent::GetLastKnownPlayerPos() const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->GetLastKnownPlayerPos();
	}
	return StaticZeroVector;
}

const FVector& UNpcContextComponent::GetLastHeardSoundLocation() const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->GetLastHeardSoundLocation();
	}
	return StaticZeroVector;
}

const FNpcFocusTarget& UNpcContextComponent::GetCurrentFocus() const
{
	if (const UNpcFocusComponent* FocusComp = GetFocusComponent())
	{
		return FocusComp->GetCurrentFocus();
	}
	return StaticEmptyFocusTarget;
}

bool UNpcContextComponent::IsDistracted() const
{
	if (const UNpcFocusComponent* FocusComp = GetFocusComponent())
	{
		return FocusComp->IsDistracted();
	}
	return false;
}

bool UNpcContextComponent::RequestFocus(const FNpcFocusTarget& NewFocusCandidate)
{
	if (UNpcFocusComponent* FocusComp = GetFocusComponent())
	{
		return FocusComp->RequestFocus(NewFocusCandidate);
	}
	return false;
}

void UNpcContextComponent::ClearFocus(ENpcFocusPriority MinimumPriorityToClear)
{
	if (UNpcFocusComponent* FocusComp = GetFocusComponent())
	{
		FocusComp->ClearFocus(MinimumPriorityToClear);
	}
}

void UNpcContextComponent::OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus)
{
	if (UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		SuspicionComp->OnSightStimulus(Actor, Stimulus);
	}
}

bool UNpcContextComponent::OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->OnHearingStimulus(Actor, Stimulus);
	}
	return false;
}

void UNpcContextComponent::HandleCrimeReported(const FAiCrimeEventPayload& CrimePayload)
{
	if (UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		SuspicionComp->HandleCrimeReported(CrimePayload);
	}

	if (CrimePayload.bIsPrimaryInvestigator)
	{
		SetNpcState(StealthAiTags::TAG_NPC_State_Combat);
	}
	else
	{
		SetNpcState(StealthAiTags::TAG_NPC_State_Alerted);
	}

	SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_CrimeReported);
}

void UNpcContextComponent::AddSuspicion(float Amount)
{
	if (UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		SuspicionComp->AddSuspicion(Amount);
	}
}

void UNpcContextComponent::SetAlertLevel(ENpcAlertLevel NewAlertLevel)
{
	if (UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		SuspicionComp->SetAlertLevel(NewAlertLevel);
	}

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

void UNpcContextComponent::SetBehaviourState(ENpcBehaviourState NewState)
{
	if (UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		SuspicionComp->SetBehaviourState(NewState);
	}

	switch (NewState)
	{
	case ENpcBehaviourState::Combat:
		SetNpcState(StealthAiTags::TAG_NPC_State_Combat);
		break;
	case ENpcBehaviourState::Search:
		SetNpcState(StealthAiTags::TAG_NPC_State_Search);
		break;
	case ENpcBehaviourState::Alerted:
		SetNpcState(StealthAiTags::TAG_NPC_State_Alerted);
		break;
	case ENpcBehaviourState::Suspicious:
		SetNpcState(StealthAiTags::TAG_NPC_State_Suspicious);
		break;
	case ENpcBehaviourState::Routine:
	default:
		SetNpcState(StealthAiTags::TAG_NPC_State_Unaware);
		break;
	}
}

void UNpcContextComponent::SetNpcState(const FGameplayTag& NewStateTag)
{
	if (CurrentStateTag == NewStateTag || !NewStateTag.IsValid())
	{
		return;
	}

	const FGameplayTag PreviousStateTag = CurrentStateTag;
	CurrentStateTag = NewStateTag;

	UE_LOG(LogStealth, Warning, TEXT("NpcContextComponent: New State Tag: %s"), *NewStateTag.GetTagName().ToString())

	// Synchronize subcomponents
	if (UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		if (NewStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Combat) || NewStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Dead))
		{
			SuspicionComp->SetAlertLevel(ENpcAlertLevel::Hostile);
		}
		else if (NewStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Alerted))
		{
			SuspicionComp->SetAlertLevel(ENpcAlertLevel::Alerted);
		}
		else if (NewStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Search))
		{
			SuspicionComp->SetBehaviourState(ENpcBehaviourState::Search);
		}
		else if (NewStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Suspicious))
		{
			SuspicionComp->SetAlertLevel(ENpcAlertLevel::Suspicious);
		}
		else
		{
			SuspicionComp->SetAlertLevel(ENpcAlertLevel::Unaware);
		}
	}

	// 1. Notify StateTree directly so behavior transitions immediately
	SendStateTreeEvent(CurrentStateTag);

	if (PreviousStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Search) && NewStateTag.MatchesTagExact(StealthAiTags::TAG_NPC_State_Unaware))
	{
		SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_SearchExpired);
		SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_ResumeRoutine);
	}

	// 2. Broadcast internal delegates for local listeners (UI, AnimInstance, Character)
	OnNpcStateChanged.Broadcast(CurrentStateTag, PreviousStateTag);
	OnAlertLevelChanged.Broadcast(GetAlertLevel());
	OnBehaviourStateChanged.Broadcast(GetBehaviourState());

	// 3. Broadcast global message via GameplayMessageSubsystem for loose coupling across systems
	if (UWorld* World = GetWorld())
	{
		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(World);
		FNpcStateChangedMessage Msg;
		Msg.NpcGUID = OwnerNpcGuid;
		Msg.PreviousStateTag = PreviousStateTag;
		Msg.NewStateTag = CurrentStateTag;
		Msg.Suspicion = GetAwareness();
		MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_NPC_StateChanged, Msg);
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

void UNpcContextComponent::HandleAlertStateEvaluated(const FGameplayTag& TargetStateTag, ENpcAlertLevel NewAlertLevel)
{
	SetNpcState(TargetStateTag);
}

void UNpcContextComponent::HandleSuspicionChanged(float NewSuspicion)
{
	OnSuspicionChanged.Broadcast(NewSuspicion);
}

void UNpcContextComponent::HandlePlayerInSightChanged(bool bInSight)
{
	const AStealthPlayerCharacter* Player = GetPlayerCharacter();
	const bool bIsIllegal = Player && IsPlayerPerformingIllegalAction(Player);
	const ENpcBehaviourState CurrentState = GetBehaviourState();
	const ENpcAlertLevel CurrentAlert = GetAlertLevel();

	if (bInSight)
	{
		// Only fire PlayerSpotted if the NPC is in combat/search (reacquiring target),
		// if the player is actively committing an illegal act, or if NPC is hostile
		if (CurrentState == ENpcBehaviourState::Combat ||
		    CurrentState == ENpcBehaviourState::Search ||
		    CurrentAlert == ENpcAlertLevel::Hostile ||
		    bIsIllegal)
		{
			SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_PlayerSpotted);
		}
	}
	else
	{
		// Only fire PlayerLost if the NPC was actively in combat, search, or hostile
		if (CurrentState == ENpcBehaviourState::Combat ||
		    CurrentState == ENpcBehaviourState::Search ||
		    CurrentAlert == ENpcAlertLevel::Hostile)
		{
			SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_PlayerLost);
		}
	}

	OnPlayerInSightChanged.Broadcast(bInSight);
}

void UNpcContextComponent::HandleFocusChanged(const FNpcFocusTarget& NewFocus, const FNpcFocusTarget& PreviousFocus)
{
	if (NewFocus.IsValid() && NewFocus.Priority >= ENpcFocusPriority::MinorDistraction)
	{
		SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_Distraction);
	}
	else if (!NewFocus.IsValid() && PreviousFocus.IsValid())
	{
		SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_FocusCleared);
		SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_ResumeRoutine);
	}

	OnNpcFocusChanged.Broadcast(NewFocus, PreviousFocus);
}

void UNpcContextComponent::HandleNoiseHeard(ENpcNoiseType NoiseType, const FVector& NoiseLocation)
{
	SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_NoiseHeard);
	OnNoiseHeard.Broadcast(NoiseType, NoiseLocation);
}
