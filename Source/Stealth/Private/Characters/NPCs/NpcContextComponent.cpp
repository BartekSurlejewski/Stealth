#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/AI/States/NpcState.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcCharacter.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/AI/Suspicion/NpcSuspicionComponent.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Legality/LegalitySubsystem.h"
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
		SuspicionComponent->OnNpcStateChanged.RemoveDynamic(this, &UNpcContextComponent::HandleNpcStateChanged);
		SuspicionComponent->OnSuspicionChanged.RemoveDynamic(this, &UNpcContextComponent::HandleSuspicionChanged);
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

		SuspicionComp->OnNpcStateChanged.AddUniqueDynamic(this, &UNpcContextComponent::HandleNpcStateChanged);
		SuspicionComp->OnSuspicionChanged.AddUniqueDynamic(this, &UNpcContextComponent::HandleSuspicionChanged);
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

bool UNpcContextComponent::IsPlayerPerformingIllegalAction() const
{
	if (const ULegalitySubsystem* Legality = ULegalitySubsystem::Get(GetWorld()))
	{
		return Legality->IsPlayerPerformingIllegalAction();
	}

	return false;
}

FGameplayTag UNpcContextComponent::GetCurrentStateTag() const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->GetCurrentStateTag();
	}
	return StealthAiTags::TAG_NPC_State_Unaware;
}

float UNpcContextComponent::GetSuspicion() const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->GetSuspicion();
	}
	return 0.0f;
}

float UNpcContextComponent::GetAwareness() const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->GetAwareness();
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
	SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_CrimeReported);
}

void UNpcContextComponent::AddSuspicion(float Amount)
{
	if (UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		SuspicionComp->AddSuspicion(Amount);
	}
}

UNpcState* UNpcContextComponent::GetCurrentState() const
{
	if (const UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		return SuspicionComp->GetCurrentState();
	}
	return nullptr;
}

void UNpcContextComponent::SetAlertLevel(ENpcAlertLevel NewAlertLevel)
{
	if (UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		SuspicionComp->SetAlertLevel(NewAlertLevel);
	}
}

void UNpcContextComponent::SetBehaviourState(ENpcBehaviourState NewState)
{
	if (UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		SuspicionComp->SetBehaviourState(NewState);
	}
}

void UNpcContextComponent::SetNpcState(const FGameplayTag& NewStateTag)
{
	if (UNpcSuspicionComponent* SuspicionComp = GetSuspicionComponent())
	{
		SuspicionComp->TransitionToStateByTag(NewStateTag);
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

void UNpcContextComponent::HandleNpcStateChanged(const FGameplayTag& NewStateTag, const FGameplayTag& PreviousStateTag)
{
	CurrentStateTag = NewStateTag;

	// Broadcast internal delegates for local listeners (UI, AnimInstance, Character)
	OnNpcStateChanged.Broadcast(CurrentStateTag, PreviousStateTag);
	OnAlertLevelChanged.Broadcast(GetAlertLevel());
	OnBehaviourStateChanged.Broadcast(GetBehaviourState());

	// Broadcast global message via GameplayMessageSubsystem
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

void UNpcContextComponent::HandleSuspicionChanged(float NewSuspicion)
{
	OnSuspicionChanged.Broadcast(NewSuspicion);
}

void UNpcContextComponent::HandlePlayerInSightChanged(bool bInSight)
{
	const AStealthPlayerCharacter* Player = GetPlayerCharacter();
	const bool bIsIllegal = Player && IsPlayerPerformingIllegalAction();
	const ENpcBehaviourState CurrentState = GetBehaviourState();
	const ENpcAlertLevel CurrentAlert = GetAlertLevel();

	if (bInSight)
	{
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
