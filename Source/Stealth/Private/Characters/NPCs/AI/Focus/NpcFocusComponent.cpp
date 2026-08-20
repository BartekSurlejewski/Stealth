#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/NpcAiController.h"
#include "GameFramework/Pawn.h"

UNpcFocusComponent::UNpcFocusComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 0.05f;
}

void UNpcFocusComponent::BeginPlay()
{
	Super::BeginPlay();
	GetAiController();
}

void UNpcFocusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateFocus(DeltaTime);
}

ANpcAiController* UNpcFocusComponent::GetAiController() const
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

bool UNpcFocusComponent::RequestFocus(const FNpcFocusTarget& NewFocusCandidate)
{
	// Rule: Higher numerical priority wins.
	// If incoming priority is lower than active non-expired focus, reject.
	if (NewFocusCandidate.Priority < CurrentFocus.Priority && CurrentFocus.RemainingTime > 0.0f)
	{
		return false;
	}

	const FNpcFocusTarget PreviousFocus = CurrentFocus;
	CurrentFocus = NewFocusCandidate;
	CurrentFocus.RemainingTime = NewFocusCandidate.Duration;

	ApplyPhysicalFocus();
	OnNpcFocusChanged.Broadcast(CurrentFocus, PreviousFocus);
	return true;
}

void UNpcFocusComponent::ClearFocus(ENpcFocusPriority MinimumPriorityToClear)
{
	if (CurrentFocus.Priority <= MinimumPriorityToClear || MinimumPriorityToClear == ENpcFocusPriority::None)
	{
		if (CurrentFocus.Priority == ENpcFocusPriority::None && !CurrentFocus.FocusActor.IsValid() && CurrentFocus.FocusLocation.IsZero())
		{
			return;
		}

		const FNpcFocusTarget PreviousFocus = CurrentFocus;
		CurrentFocus = FNpcFocusTarget();

		if (ANpcAiController* Controller = GetAiController())
		{
			Controller->ClearFocus(EAIFocusPriority::Gameplay);
		}

		OnNpcFocusChanged.Broadcast(CurrentFocus, PreviousFocus);
	}
}

void UNpcFocusComponent::UpdateFocus(float DeltaTime)
{
	if (CurrentFocus.Duration > 0.0f && CurrentFocus.RemainingTime > 0.0f)
	{
		CurrentFocus.RemainingTime -= DeltaTime;
		if (CurrentFocus.RemainingTime <= 0.0f)
		{
			ClearFocus(CurrentFocus.Priority);
		}
	}
}

void UNpcFocusComponent::ApplyPhysicalFocus()
{
	ANpcAiController* Controller = GetAiController();
	if (!Controller)
	{
		return;
	}

	if (CurrentFocus.FocusActor.IsValid())
	{
		Controller->SetFocus(CurrentFocus.FocusActor.Get(), EAIFocusPriority::Gameplay);
	}
	else if (!CurrentFocus.FocusLocation.IsZero())
	{
		Controller->SetFocalPoint(CurrentFocus.FocusLocation, EAIFocusPriority::Gameplay);
	}
	else
	{
		Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}

bool UNpcFocusComponent::IsDistracted() const
{
	return CurrentFocus.Priority >= ENpcFocusPriority::MinorDistraction && CurrentFocus.Priority < ENpcFocusPriority::SuspiciousPlayer;
}

float UNpcFocusComponent::GetAwarenessMultiplier() const
{
	if (IsDistracted())
	{
		return FMath::Clamp(CurrentFocus.AwarenessReductionMultiplier, 0.0f, 1.0f);
	}
	return 1.0f;
}
