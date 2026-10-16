#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/AI/Perception/NpcSensorComponent.h"
#include "Characters/NPCs/Guards/NpcProfile.h"

UNpcFocusComponent::UNpcFocusComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UNpcFocusComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const ANpcAiController* Controller = GetNpcController())
	{
		if (UNpcSensorComponent* Sensor = Controller->GetSensorComponent())
		{
			Sensor->OnSensorEvent.AddUObject(this, &UNpcFocusComponent::HandleSensorEvent);
		}
	}
}

void UNpcFocusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentFocus.IsValid() && CurrentFocus.Duration > 0.0f)
	{
		CurrentFocus.RemainingTime -= DeltaTime;
		if (CurrentFocus.RemainingTime <= 0.0f)
		{
			ClearFocus(CurrentFocus.Priority);
		}
	}
}

bool UNpcFocusComponent::RequestFocus(const FNpcFocusTarget& NewFocusCandidate)
{
	if (!NewFocusCandidate.IsValid())
	{
		return false;
	}

	if (CurrentFocus == NewFocusCandidate)
	{
		return true;
	}

	const bool bHasActiveProtectedFocus = CurrentFocus.IsValid() &&
		(CurrentFocus.Duration <= 0.0f || CurrentFocus.RemainingTime > 0.0f);

	if (bHasActiveProtectedFocus)
	{
		if (NewFocusCandidate.Priority <= CurrentFocus.Priority)
		{
			return false;
		}
	}

	const FNpcFocusTarget PreviousFocus = CurrentFocus;
	CurrentFocus = NewFocusCandidate;
	CurrentFocus.RemainingTime = NewFocusCandidate.Duration;

	ApplyPhysicalFocus();
	OnNpcFocusChanged.Broadcast(CurrentFocus, PreviousFocus);
	return true;
}

void UNpcFocusComponent::ClearFocus(ENpcFocusPriority MaxPriorityToClear)
{
	if (!CurrentFocus.IsValid())
	{
		return;
	}

	if (MaxPriorityToClear == ENpcFocusPriority::None || CurrentFocus.Priority <= MaxPriorityToClear)
	{
		const FNpcFocusTarget PreviousFocus = CurrentFocus;
		CurrentFocus = FNpcFocusTarget();

		if (ANpcAiController* Controller = GetNpcController())
		{
			Controller->ClearFocus(EAIFocusPriority::Gameplay);
		}

		OnNpcFocusChanged.Broadcast(CurrentFocus, PreviousFocus);
	}
}

bool UNpcFocusComponent::IsFocusedOn(const AActor* Actor) const
{
	return Actor && CurrentFocus.FocusActor.Get() == Actor;
}

bool UNpcFocusComponent::IsFocusedOnPlayer() const
{
	const AActor* Actor = CurrentFocus.FocusActor.Get();
	return Actor && NpcAi::IsPlayerActor(Actor);
}

bool UNpcFocusComponent::IsDistracted() const
{
	if (IsFocusedOnPlayer())
	{
		return false;
	}
	return CurrentFocus.Priority >= ENpcFocusPriority::MinorDistraction && CurrentFocus.Priority < ENpcFocusPriority::SuspiciousPlayer;
}

float UNpcFocusComponent::GetAwarenessMultiplier() const
{
	if (CurrentFocus.IsValid())
	{
		return FMath::Clamp(CurrentFocus.AwarenessReductionMultiplier, 0.0f, 1.0f);
	}
	return 1.0f;
}

void UNpcFocusComponent::ApplyPhysicalFocus()
{
	ANpcAiController* Controller = GetNpcController();
	if (!Controller)
	{
		return;
	}

	if (CurrentFocus.FocusActor.IsValid())
	{
		Controller->SetFocus(CurrentFocus.FocusActor.Get(), EAIFocusPriority::Gameplay);
	}
	else if (CurrentFocus.bHasLocation)
	{
		Controller->SetFocalPoint(CurrentFocus.FocusLocation, EAIFocusPriority::Gameplay);
	}
	else
	{
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}

void UNpcFocusComponent::HandleSensorEvent(const FNpcSensorEvent& Event)
{
	const UNpcProfile* Profile = GetNpcProfile();
	if (!Profile)
	{
		return;
	}

	switch (Event.Type)
	{
	case ENpcSensorEventType::SightGained:
		if (Event.Actor.IsValid() && NpcAi::IsPlayerActor(Event.Actor.Get()))
		{
			const FNpcFocusRule& Rule = Profile->PlayerSightedFocusRule;
			if (Rule.Priority > ENpcFocusPriority::None)
			{
				FNpcFocusTarget Candidate;
				Candidate.FocusTag = Rule.FocusTag;
				Candidate.Priority = Rule.Priority;
				Candidate.FocusActor = Event.Actor;
				Candidate.FocusLocation = Event.Location;
				Candidate.bHasLocation = true;
				Candidate.Duration = Rule.Duration;
				Candidate.AwarenessReductionMultiplier = Rule.AwarenessReductionMultiplier;
				RequestFocus(Candidate);
			}
		}
		break;

	case ENpcSensorEventType::SightLost:
		// On SightLost keep the focus (awareness handles staleness)
		break;

	case ENpcSensorEventType::Heard:
		if (const FNpcFocusRule* Rule = Profile->NoiseFocusRules.Find(Event.NoiseType))
		{
			if (Rule->Priority > ENpcFocusPriority::None)
			{
				FNpcFocusTarget Candidate;
				Candidate.FocusTag = Rule->FocusTag;
				Candidate.Priority = Rule->Priority;
				Candidate.FocusActor = Event.Actor;
				Candidate.FocusLocation = Event.Location;
				Candidate.bHasLocation = true;
				Candidate.Duration = Rule->Duration;
				Candidate.AwarenessReductionMultiplier = Rule->AwarenessReductionMultiplier;
				RequestFocus(Candidate);
			}
		}
		break;

	case ENpcSensorEventType::Touched:
		if (Event.Actor.IsValid() && NpcAi::IsPlayerActor(Event.Actor.Get()))
		{
			const FNpcFocusRule& Rule = Profile->PlayerTouchedFocusRule;
			if (Rule.Priority > ENpcFocusPriority::None)
			{
				FNpcFocusTarget Candidate;
				Candidate.FocusTag = Rule.FocusTag;
				Candidate.Priority = Rule.Priority;
				Candidate.FocusActor = Event.Actor;
				Candidate.FocusLocation = Event.Location;
				Candidate.bHasLocation = true;
				Candidate.Duration = Rule.Duration;
				Candidate.AwarenessReductionMultiplier = Rule.AwarenessReductionMultiplier;
				RequestFocus(Candidate);
			}
		}
		break;

	case ENpcSensorEventType::Damaged:
		if (Event.Actor.IsValid() && NpcAi::IsPlayerActor(Event.Actor.Get()))
		{
			const FNpcFocusRule& Rule = Profile->PlayerDamagedFocusRule;
			if (Rule.Priority > ENpcFocusPriority::None)
			{
				FNpcFocusTarget Candidate;
				Candidate.FocusTag = Rule.FocusTag;
				Candidate.Priority = Rule.Priority;
				Candidate.FocusActor = Event.Actor;
				Candidate.FocusLocation = Event.Location;
				Candidate.bHasLocation = true;
				Candidate.Duration = Rule.Duration;
				Candidate.AwarenessReductionMultiplier = Rule.AwarenessReductionMultiplier;
				RequestFocus(Candidate);
			}
		}
		break;

	case ENpcSensorEventType::Forgotten:
		if (Event.Actor.IsValid() && CurrentFocus.FocusActor == Event.Actor)
		{
			ClearFocus(CurrentFocus.Priority);
		}
		break;
	}
}
