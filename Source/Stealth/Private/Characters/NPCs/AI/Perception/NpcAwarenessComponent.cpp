#include "Characters/NPCs/AI/Perception/NpcAwarenessComponent.h"
#include "Characters/NPCs/AI/Perception/NpcSensorComponent.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "HAL/IConsoleManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"

static TAutoConsoleVariable<int32> CVarDebugNpcPerception(
	TEXT("stealth.npc.debugperception"),
	0,
	TEXT("Enable on-screen and visual debug rendering for NPC perception.\n")
	TEXT("0 = disabled, 1 = enabled"),
	ECVF_Cheat
);

UNpcAwarenessComponent::UNpcAwarenessComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UNpcAwarenessComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UNpcSensorComponent* Sensor = GetSensorComponent())
	{
		AddTickPrerequisiteComponent(Sensor);
	}
}

void UNpcAwarenessComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateAwareness(DeltaTime);
}

ANpcAiController* UNpcAwarenessComponent::GetAiController() const
{
	if (!CachedController.IsValid())
	{
		CachedController = Cast<ANpcAiController>(GetOwner());
		if (!CachedController.IsValid())
		{
			if (const APawn* Pawn = Cast<APawn>(GetOwner()))
			{
				CachedController = Cast<ANpcAiController>(Pawn->GetController());
			}
		}
	}
	return CachedController.Get();
}

UNpcSensorComponent* UNpcAwarenessComponent::GetSensorComponent() const
{
	if (!CachedSensorComponent.IsValid())
	{
		if (const ANpcAiController* Controller = GetAiController())
		{
			CachedSensorComponent = Controller->FindComponentByClass<UNpcSensorComponent>();
		}
		if (!CachedSensorComponent.IsValid())
		{
			CachedSensorComponent = GetOwner()->FindComponentByClass<UNpcSensorComponent>();
		}
	}
	return CachedSensorComponent.Get();
}

UNpcFocusComponent* UNpcAwarenessComponent::GetFocusComponent() const
{
	if (!CachedFocusComponent.IsValid())
	{
		if (const ANpcAiController* Controller = GetAiController())
		{
			CachedFocusComponent = Controller->GetFocusComponent();
		}
		if (!CachedFocusComponent.IsValid())
		{
			CachedFocusComponent = GetOwner()->FindComponentByClass<UNpcFocusComponent>();
		}
	}
	return CachedFocusComponent.Get();
}

UNpcContextComponent* UNpcAwarenessComponent::GetContextComponent() const
{
	if (!CachedContextComponent.IsValid())
	{
		if (const ANpcAiController* Controller = GetAiController())
		{
			CachedContextComponent = Controller->FindComponentByClass<UNpcContextComponent>();
		}
		if (!CachedContextComponent.IsValid())
		{
			CachedContextComponent = GetOwner()->FindComponentByClass<UNpcContextComponent>();
		}
	}
	return CachedContextComponent.Get();
}

const UNpcProfile* UNpcAwarenessComponent::GetNpcProfile() const
{
	if (const UNpcSensorComponent* Sensor = GetSensorComponent())
	{
		return Sensor->GetNpcProfile();
	}
	if (const UNpcContextComponent* Context = GetContextComponent())
	{
		return Context->GetNpcProfile();
	}
	return nullptr;
}

void UNpcAwarenessComponent::SetPerceptionTuning(const FNpcPerceptionTuningParams& InParams)
{
	TuningParams = InParams;
}

void UNpcAwarenessComponent::SetPerceptionTuningDirect(ENpcSuspicionMode InMode, float InGainMultiplier, float InDecayMultiplier)
{
	TuningParams.Mode = InMode;
	TuningParams.SuspicionGainMultiplier = InGainMultiplier;
	TuningParams.SuspicionDecayMultiplier = InDecayMultiplier;
}

void UNpcAwarenessComponent::SetSuspicion(float NewSuspicion)
{
	const float ClampedSuspicion = FMath::Clamp(NewSuspicion, 0.0f, 100.0f);
	if (!FMath::IsNearlyEqual(CurrentSuspicion, ClampedSuspicion))
	{
		CurrentSuspicion = ClampedSuspicion;
		OnSuspicionChanged.Broadcast(CurrentSuspicion);
	}
}

void UNpcAwarenessComponent::AddSuspicion(float Delta)
{
	SetSuspicion(CurrentSuspicion + Delta);
}

void UNpcAwarenessComponent::UpdateAwareness(float DeltaTime)
{
	UpdateVisibilityGrade();
	UpdateSuspicion(DeltaTime);

	// Update memory staleness
	if (bIsPlayerVisible)
	{
		LostPlayerSightDuration = 0.0f;
	}
	else
	{
		LostPlayerSightDuration += DeltaTime;
	}

	const UNpcProfile* Profile = GetNpcProfile();
	const float GracePeriod = Profile ? Profile->LosePlayerSightGracePeriod : 1.0f;
	bMemoryStale = (LostPlayerSightDuration >= GracePeriod);

	UpdateInvestigationTarget();
	BuildPerceptionSnapshot();

	if (CVarDebugNpcPerception.GetValueOnGameThread() > 0)
	{
		DrawDebugVisuals();
	}
}

void UNpcAwarenessComponent::UpdateVisibilityGrade()
{
	UNpcSensorComponent* Sensor = GetSensorComponent();
	if (!Sensor)
	{
		VisibilityGrade = ENpcVisibilityGrade::Hidden;
		bIsPlayerVisible = false;
		return;
	}

	const UNpcProfile* Profile = GetNpcProfile();
	const float ShadowThreshold = Profile ? Profile->VisibilityExposureThreshold_Shadowed : 0.15f;
	const float FullyVisibleThreshold = Profile ? Profile->VisibilityExposureThreshold_FullyVisible : 0.6f;

	const bool bLineOfSightValid = Sensor->IsLineOfSightValid();
	const bool bIsFullyVisibleByDistance = Sensor->IsFullyVisibleByDistance();
	const float CurrentExposure = Sensor->GetPlayerExposureMultiplier();
	const bool bTouch = Sensor->WasTouchSensedThisTick();

	if (bTouch)
	{
		VisibilityGrade = ENpcVisibilityGrade::FullyVisible;
		bIsPlayerVisible = true;
	}
	else if (!bLineOfSightValid)
	{
		VisibilityGrade = ENpcVisibilityGrade::Hidden;
		bIsPlayerVisible = false;
	}
	else if (bIsFullyVisibleByDistance || CurrentExposure >= FullyVisibleThreshold)
	{
		VisibilityGrade = ENpcVisibilityGrade::FullyVisible;
		bIsPlayerVisible = true;
	}
	else if (CurrentExposure >= ShadowThreshold)
	{
		VisibilityGrade = ENpcVisibilityGrade::PartiallyExposed;
		bIsPlayerVisible = true;
	}
	else
	{
		VisibilityGrade = ENpcVisibilityGrade::Shadowed;
		const float SuspiciousThreshold = Profile ? Profile->SuspicionThreshold_Suspicious : 25.0f;
		bIsPlayerVisible = (CurrentSuspicion >= SuspiciousThreshold);
	}
}

void UNpcAwarenessComponent::UpdateSuspicion(float DeltaTime)
{
	const UNpcSensorComponent* Sensor = GetSensorComponent();
	if (!Sensor)
	{
		return;
	}

	const UNpcProfile* Profile = GetNpcProfile();
	const float BaseGainRateSight = Profile ? Profile->SuspicionGainPerSecond_Sight : 40.0f;
	const float BaseGainRatePeripheral = Profile ? Profile->SuspicionGainPerSecond_Peripheral : 15.0f;
	const float BaseDecayRate = Profile ? Profile->SuspicionDecayPerSecond : 8.0f;

	const bool bHasLineOfSight = Sensor->HasLineOfSight();
	const bool bIsLookingDirectlyAtPlayer = Sensor->IsLookingDirectlyAtPlayer();
	const bool bIsPlayerPerformingIllegalAction = Sensor->IsPlayerPerformingIllegalAction();
	const float PlayerExposureMultiplier = Sensor->GetPlayerExposureMultiplier();

	UNpcFocusComponent* FocusComp = GetFocusComponent();
	const bool bIsFocusingOnPlayer = FocusComp && FocusComp->IsFocusingOnPlayer();
	const float FocusAwarenessMultiplier = FocusComp ? FocusComp->GetFocusAwarenessMultiplier() : 1.0f;

	if (Sensor->WasDamageSensedThisTick())
	{
		SetSuspicion(100.0f);
		return;
	}

	if (Sensor->WasTouchSensedThisTick())
	{
		AddSuspicion(75.0f);
	}

	float Delta = 0.0f;

	switch (TuningParams.Mode)
	{
	case ENpcSuspicionMode::Normal:
		if (bIsFocusingOnPlayer && bHasLineOfSight && bIsPlayerPerformingIllegalAction)
		{
			const float Rate = bIsLookingDirectlyAtPlayer ? BaseGainRateSight : BaseGainRatePeripheral;
			Delta = Rate * TuningParams.SuspicionGainMultiplier * FocusAwarenessMultiplier * PlayerExposureMultiplier * DeltaTime;
		}
		else
		{
			Delta = -BaseDecayRate * TuningParams.SuspicionDecayMultiplier * DeltaTime;
		}
		break;

	case ENpcSuspicionMode::FrozenWhileVisible:
		Delta = bHasLineOfSight ? 0.0f : -BaseDecayRate * TuningParams.SuspicionDecayMultiplier * DeltaTime;
		break;

	case ENpcSuspicionMode::AlwaysDecay:
		Delta = -BaseDecayRate * TuningParams.SuspicionDecayMultiplier * DeltaTime;
		break;
	}

	if (Delta != 0.0f)
	{
		AddSuspicion(Delta);
	}
}

void UNpcAwarenessComponent::UpdateInvestigationTarget()
{
	UNpcSensorComponent* Sensor = GetSensorComponent();
	if (!Sensor)
	{
		return;
	}

	const FVector LastPlayerPos = Sensor->GetLastKnownPlayerPos();
	const FVector LastSoundPos = Sensor->GetLastHeardSoundLocation();

	if (LastPlayerPos.IsZero() && LastSoundPos.IsZero())
	{
		InvestigationTarget = FVector::ZeroVector;
		return;
	}

	if (!LastPlayerPos.IsZero() && LastSoundPos.IsZero())
	{
		InvestigationTarget = LastPlayerPos;
		return;
	}

	if (LastPlayerPos.IsZero() && !LastSoundPos.IsZero())
	{
		InvestigationTarget = LastSoundPos;
		return;
	}

	// Priority: Whichever stimulus was observed more recently
	const float LastSightTime = Sensor->GetLastValidObservationTime();
	const float LastSoundTime = Sensor->GetLastHeardSoundTime();

	if (LastSightTime >= LastSoundTime)
	{
		InvestigationTarget = LastPlayerPos;
	}
	else
	{
		InvestigationTarget = LastSoundPos;
	}
}

void UNpcAwarenessComponent::BuildPerceptionSnapshot()
{
	UNpcSensorComponent* Sensor = GetSensorComponent();
	UNpcContextComponent* Context = GetContextComponent();

	PerceptionSnapshot.PlayerPosition = (Sensor && Sensor->IsLineOfSightValid()) ? Sensor->GetLastKnownPlayerPos() : FVector::ZeroVector;
	PerceptionSnapshot.LastKnownPlayerPosition = Sensor ? Sensor->GetLastKnownPlayerPos() : FVector::ZeroVector;
	PerceptionSnapshot.bIsPlayerVisible = bIsPlayerVisible;
	PerceptionSnapshot.bLineOfSightValid = Sensor ? Sensor->IsLineOfSightValid() : false;
	PerceptionSnapshot.bLineOfSightBlocked = Sensor ? Sensor->IsLineOfSightBlocked() : false;
	PerceptionSnapshot.HearingStimulusStrength = Sensor ? Sensor->GetHearingStimulusStrength() : 0.0f;
	PerceptionSnapshot.Suspicion = CurrentSuspicion;
	PerceptionSnapshot.InvestigationTarget = InvestigationTarget;
	PerceptionSnapshot.LastValidObservationTime = Sensor ? Sensor->GetLastValidObservationTime() : 0.0f;
	PerceptionSnapshot.CurrentExposure = Sensor ? Sensor->GetPlayerExposureMultiplier() : 0.0f;
	PerceptionSnapshot.VisibilityGrade = VisibilityGrade;
	PerceptionSnapshot.DistanceToPlayer = Sensor ? Sensor->GetPlayerDistance() : 0.0f;
	PerceptionSnapshot.bInDetectionRange = Sensor ? Sensor->IsInDetectionRange() : false;
	PerceptionSnapshot.bMemoryStale = bMemoryStale;
	PerceptionSnapshot.bIsFullyVisibleByDistance = Sensor ? Sensor->IsFullyVisibleByDistance() : false;
	PerceptionSnapshot.LastHeardSoundLocation = Sensor ? Sensor->GetLastHeardSoundLocation() : FVector::ZeroVector;
	PerceptionSnapshot.TimeSinceLastStimulus = Sensor ? Sensor->GetTimeSinceLastStimulus() : 0.0f;
	PerceptionSnapshot.LastPerceivedActor = Sensor ? Sensor->GetLastPerceivedActor() : nullptr;
	PerceptionSnapshot.LastStimulusTag = Sensor ? Sensor->GetLastStimulusTag() : FGameplayTag::EmptyTag;

	// Legacy backward compatibility
	PerceptionSnapshot.Awareness = CurrentSuspicion;
	PerceptionSnapshot.AlertLevel = Context ? Context->GetAlertLevel() : ENpcAlertLevel::Unaware;
	PerceptionSnapshot.bHasPlayerLineOfSight = PerceptionSnapshot.bLineOfSightValid;
	PerceptionSnapshot.bEffectivelySeesPlayer = PerceptionSnapshot.bIsPlayerVisible;
}

void UNpcAwarenessComponent::DrawDebugVisuals()
{
	if (!GEngine)
	{
		return;
	}

	const APawn* Pawn = nullptr;
	if (const ANpcAiController* Controller = GetAiController())
	{
		Pawn = Controller->GetPawn();
	}
	else
	{
		Pawn = Cast<APawn>(GetOwner());
	}

	if (!Pawn)
	{
		return;
	}

	const UNpcContextComponent* Context = GetContextComponent();
	const UNpcFocusComponent* FocusComp = GetFocusComponent();

	const FString StateTagStr = Context ? Context->GetCurrentStateTag().ToString() : TEXT("None");
	const FString FocusTagStr = (FocusComp && FocusComp->GetCurrentFocus().IsValid()) ? FocusComp->GetCurrentFocus().FocusTag.ToString() : TEXT("None");
	const FString VisGradeStr = UEnum::GetValueAsString(VisibilityGrade);

	const FString DebugStr = FString::Printf(
		TEXT("NPC [%s]\nSuspicion: %.1f | VisGrade: %s | Stale: %s\nDistToPlayer: %.1f | State: %s\nFocus: %s"),
		*Pawn->GetName(),
		CurrentSuspicion,
		*VisGradeStr,
		bMemoryStale ? TEXT("true") : TEXT("false"),
		PerceptionSnapshot.DistanceToPlayer,
		*StateTagStr,
		*FocusTagStr
	);

	DrawDebugString(GetWorld(), Pawn->GetActorLocation() + FVector(0, 0, 120), DebugStr, nullptr, FColor::Yellow, 0.0f, true);
}
