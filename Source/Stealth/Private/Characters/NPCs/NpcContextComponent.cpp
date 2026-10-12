#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcCharacter.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/AI/Perception/NpcSensorComponent.h"
#include "Characters/NPCs/AI/Perception/NpcAwarenessComponent.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Legality/LegalitySubsystem.h"
#include "Messages/StealthMessages.h"

namespace
{
	static const FVector StaticZeroVector = FVector::ZeroVector;
	static const FNpcFocusTarget StaticEmptyFocusTarget;
	static const FStateTreeAiPerceptionData StaticEmptyPerceptionData;
}

UNpcContextComponent::UNpcContextComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedStateTreeComponent = GetOwner()->FindComponentByClass<UStateTreeAIComponent>();
	CachedNpcAiController = Cast<ANpcAiController>(GetOwner());

	CurrentStateTag = StealthAiTags::TAG_NPC_State_Unaware;

	if (const UCharactersRegistrySubsystem* Registry = UCharactersRegistrySubsystem::Get(this))
	{
		if (CachedNpcAiController.Get())
		{
			OwnerNpcGuid = Registry->GetNpcGuidByController(CachedNpcAiController.Get());
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
	if (CachedAwarenessComponent.Get())
	{
		CachedAwarenessComponent->OnSuspicionChanged.RemoveDynamic(this, &UNpcContextComponent::HandleSuspicionChanged);
	}

	if (CachedFocusComponent.Get())
	{
		CachedFocusComponent->OnNpcFocusChanged.RemoveDynamic(this, &UNpcContextComponent::HandleFocusChanged);
	}

	Super::EndPlay(EndPlayReason);
}

void UNpcContextComponent::BindToSubcomponents()
{
	UNpcAwarenessComponent* AwarenessComp = GetAwarenessComponent();
	if (AwarenessComp)
	{
		AwarenessComp->OnSuspicionChanged.AddUniqueDynamic(this, &UNpcContextComponent::HandleSuspicionChanged);
	}

	UNpcFocusComponent* FocusComp = GetFocusComponent();
	if (FocusComp)
	{
		FocusComp->OnNpcFocusChanged.AddUniqueDynamic(this, &UNpcContextComponent::HandleFocusChanged);
	}
}

UNpcSensorComponent* UNpcContextComponent::GetSensorComponent() const
{
	if (!CachedSensorComponent.Get())
	{
		if (GetOwner())
		{
			CachedSensorComponent = GetOwner()->FindComponentByClass<UNpcSensorComponent>();
		}
		if (!CachedSensorComponent.Get() && CachedNpcAiController.Get())
		{
			CachedSensorComponent = CachedNpcAiController->FindComponentByClass<UNpcSensorComponent>();
		}
	}
	return CachedSensorComponent.Get();
}

UNpcAwarenessComponent* UNpcContextComponent::GetAwarenessComponent() const
{
	if (!CachedAwarenessComponent.Get())
	{
		if (GetOwner())
		{
			CachedAwarenessComponent = GetOwner()->FindComponentByClass<UNpcAwarenessComponent>();
		}
		if (!CachedAwarenessComponent.Get() && CachedNpcAiController.Get())
		{
			CachedAwarenessComponent = CachedNpcAiController->FindComponentByClass<UNpcAwarenessComponent>();
		}
	}
	return CachedAwarenessComponent.Get();
}

UNpcFocusComponent* UNpcContextComponent::GetFocusComponent() const
{
	if (!CachedFocusComponent.Get())
	{
		if (GetOwner())
		{
			CachedFocusComponent = GetOwner()->FindComponentByClass<UNpcFocusComponent>();
		}
		if (!CachedFocusComponent.Get() && CachedNpcAiController.Get())
		{
			CachedFocusComponent = CachedNpcAiController->FindComponentByClass<UNpcFocusComponent>();
		}
	}
	return CachedFocusComponent.Get();
}

UNpcProfile* UNpcContextComponent::GetNpcProfile() const
{
	if (Profile)
	{
		return Profile;
	}
	if (const UNpcSensorComponent* SensorComp = GetSensorComponent())
	{
		return SensorComp->GetNpcProfile();
	}
	return nullptr;
}

void UNpcContextComponent::SetProfile(UNpcProfile* InProfile)
{
	Profile = InProfile;
}

bool UNpcContextComponent::IsPlayerPerformingIllegalAction() const
{
	if (const UNpcSensorComponent* SensorComp = GetSensorComponent())
	{
		return SensorComp->IsPlayerPerformingIllegalAction();
	}

	if (const ULegalitySubsystem* Legality = ULegalitySubsystem::Get(GetWorld()))
	{
		return Legality->IsPlayerPerformingIllegalAction();
	}

	return false;
}

float UNpcContextComponent::GetSuspicion() const
{
	if (const UNpcAwarenessComponent* AwarenessComp = GetAwarenessComponent())
	{
		return AwarenessComp->GetSuspicion();
	}
	return 0.0f;
}

float UNpcContextComponent::GetAwareness() const
{
	return GetSuspicion();
}

ENpcAlertLevel UNpcContextComponent::GetAlertLevel() const
{
	return ResolveAlertLevelFromStateTag(CurrentStateTag);
}

ENpcBehaviourState UNpcContextComponent::GetBehaviourState() const
{
	return ResolveBehaviourStateFromStateTag(CurrentStateTag);
}

bool UNpcContextComponent::HasPlayerLineOfSight() const
{
	if (const UNpcSensorComponent* SensorComp = GetSensorComponent())
	{
		return SensorComp->IsLineOfSightValid();
	}
	return false;
}

bool UNpcContextComponent::EffectivelySeesPlayer() const
{
	if (const UNpcAwarenessComponent* AwarenessComp = GetAwarenessComponent())
	{
		return AwarenessComp->IsPlayerVisible();
	}
	return false;
}

FVector UNpcContextComponent::GetLastKnownPlayerPos() const
{
	if (const UNpcSensorComponent* SensorComp = GetSensorComponent())
	{
		return SensorComp->GetLastKnownPlayerPos();
	}
	return StaticZeroVector;
}

FVector UNpcContextComponent::GetLastHeardSoundLocation() const
{
	if (const UNpcSensorComponent* SensorComp = GetSensorComponent())
	{
		return SensorComp->GetLastHeardSoundLocation();
	}
	return StaticZeroVector;
}

FVector UNpcContextComponent::GetInvestigationTarget() const
{
	if (const UNpcAwarenessComponent* AwarenessComp = GetAwarenessComponent())
	{
		return AwarenessComp->GetInvestigationTarget();
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

const FStateTreeAiPerceptionData& UNpcContextComponent::GetPerceptionSnapshot() const
{
	if (const UNpcAwarenessComponent* AwarenessComp = GetAwarenessComponent())
	{
		return AwarenessComp->GetPerceptionSnapshot();
	}
	return StaticEmptyPerceptionData;
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
	if (UNpcSensorComponent* SensorComp = GetSensorComponent())
	{
		SensorComp->OnSightStimulus(const_cast<AActor*>(Actor), Stimulus);
	}
}

void UNpcContextComponent::HandleHearingStimulus(ENpcNoiseType NoiseType, const FAIStimulus& Stimulus)
{
	SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_NoiseHeard);
	OnNoiseHeard.Broadcast(NoiseType, Stimulus.StimulusLocation);
}

void UNpcContextComponent::HandleCrimeReported(const FAiCrimeEventPayload& CrimePayload)
{
	AddSuspicion(CrimePayload.bIsPrimaryInvestigator ? 100.0f : 50.0f);
	SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_CrimeReported);
}

void UNpcContextComponent::AddSuspicion(float Amount)
{
	if (UNpcAwarenessComponent* AwarenessComp = GetAwarenessComponent())
	{
		AwarenessComp->AddSuspicion(Amount);
	}
}

void UNpcContextComponent::SetSuspicion(float NewSuspicion)
{
	if (UNpcAwarenessComponent* AwarenessComp = GetAwarenessComponent())
	{
		AwarenessComp->SetSuspicion(NewSuspicion);
	}
}

void UNpcContextComponent::SetCurrentStateTag(const FGameplayTag& NewStateTag)
{
	if (CurrentStateTag == NewStateTag)
	{
		return;
	}

	const FGameplayTag PreviousStateTag = CurrentStateTag;
	CurrentStateTag = NewStateTag;

	// Broadcast internal delegates
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

void UNpcContextComponent::SetCurrentStanceTag(const FGameplayTag& NewStanceTag)
{
	CurrentStanceTag = NewStanceTag;
}

void UNpcContextComponent::SendStateTreeEvent(const FGameplayTag& Tag) const
{
	if (CachedStateTreeComponent.Get() && Tag.IsValid())
	{
		CachedStateTreeComponent->SendStateTreeEvent(FStateTreeEvent(Tag));
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

void UNpcContextComponent::HandleSuspicionChanged(float NewSuspicion)
{
	OnSuspicionChanged.Broadcast(NewSuspicion);
}

void UNpcContextComponent::HandlePlayerInSightChanged(bool bInSight)
{
	if (bInSight)
	{
		SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_PlayerSpotted);
	}
	else
	{
		SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_PlayerLost);
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

ENpcAlertLevel UNpcContextComponent::ResolveAlertLevelFromStateTag(const FGameplayTag& StateTag)
{
	if (StateTag.MatchesTag(StealthAiTags::TAG_NPC_State_Combat) || StateTag.MatchesTag(StealthAiTags::TAG_NPC_State_Fleeing))
	{
		return ENpcAlertLevel::Hostile;
	}
	if (StateTag.MatchesTag(StealthAiTags::TAG_NPC_State_Alerted) || StateTag.MatchesTag(StealthAiTags::TAG_NPC_State_Search))
	{
		return ENpcAlertLevel::Alerted;
	}
	if (StateTag.MatchesTag(StealthAiTags::TAG_NPC_State_Suspicious))
	{
		return ENpcAlertLevel::Suspicious;
	}
	return ENpcAlertLevel::Unaware;
}

ENpcBehaviourState UNpcContextComponent::ResolveBehaviourStateFromStateTag(const FGameplayTag& StateTag)
{
	if (StateTag.MatchesTag(StealthAiTags::TAG_NPC_State_Combat) || StateTag.MatchesTag(StealthAiTags::TAG_NPC_State_Fleeing))
	{
		return ENpcBehaviourState::Combat;
	}
	if (StateTag.MatchesTag(StealthAiTags::TAG_NPC_State_Search))
	{
		return ENpcBehaviourState::Search;
	}
	if (StateTag.MatchesTag(StealthAiTags::TAG_NPC_State_Alerted))
	{
		return ENpcBehaviourState::Alerted;
	}
	if (StateTag.MatchesTag(StealthAiTags::TAG_NPC_State_Suspicious))
	{
		return ENpcBehaviourState::Suspicious;
	}
	return ENpcBehaviourState::Routine;
}
