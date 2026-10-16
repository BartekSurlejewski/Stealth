#include "Characters/NPCs/AI/Perception/NpcAwarenessComponent.h"
#include "Characters/NPCs/AI/Perception/NpcSensorComponent.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Characters/NPCs/AI/Debug/NpcPerceptionDebugHelper.h"
#include "Legality/LegalitySubsystem.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Exposure/PlayerExposureSubsystem.h"

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

	if (const ANpcAiController* Controller = GetNpcController())
	{
		if (UNpcSensorComponent* Sensor = Controller->GetSensorComponent())
		{
			AddTickPrerequisiteComponent(Sensor);
			Sensor->OnSensorEvent.AddUObject(this, &UNpcAwarenessComponent::HandleSensorEvent);
		}
	}
}

void UNpcAwarenessComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateAwareness(DeltaTime);

	if (CVarDebugNpcPerception.GetValueOnGameThread() > 0)
	{
		if (const ANpcAiController* Controller = GetNpcController())
		{
			NpcAiDebug::DrawPerceptionDebug(this, Controller->GetFocusComponent(), Controller->GetContextComponent(), GetNpcPawn());
		}
	}
}

void UNpcAwarenessComponent::SetPerceptionTuning(const FNpcPerceptionTuningParams& InParams)
{
	TuningParams = InParams;
}

void UNpcAwarenessComponent::SetSuspicion(float NewSuspicion)
{
	const float ClampedSuspicion = FMath::Clamp(NewSuspicion, 0.0f, 100.0f);
	AwarenessState.Suspicion = ClampedSuspicion;

	const UNpcProfile* Profile = GetNpcProfile();
	const float Threshold = Profile ? Profile->SuspicionThreshold_Suspicious : 25.0f;

	const bool bIsAbove = (AwarenessState.Suspicion >= Threshold);
	if (bIsAbove && !bWasAboveSuspiciousThreshold)
	{
		OnSuspiciousThresholdCrossed.Broadcast(AwarenessState.Suspicion);
	}
	bWasAboveSuspiciousThreshold = bIsAbove;
}

void UNpcAwarenessComponent::AddSuspicion(float Delta)
{
	SetSuspicion(AwarenessState.Suspicion + Delta);
}

void UNpcAwarenessComponent::HandleSensorEvent(const FNpcSensorEvent& Event)
{
	const UNpcProfile* Profile = GetNpcProfile();
	if (!Profile)
	{
		return;
	}

	if (Event.Type == ENpcSensorEventType::Damaged)
	{
		SetSuspicion(Profile->DamageSuspicion);
	}
	else if (Event.Type == ENpcSensorEventType::Touched)
	{
		AddSuspicion(Profile->TouchSuspicionGain);
	}
}

bool UNpcAwarenessComponent::IsFocusTargetThreatening(const AActor* TargetActor) const
{
	if (!TargetActor)
	{
		return false;
	}

	if (NpcAi::IsPlayerActor(TargetActor))
	{
		if (const ULegalitySubsystem* Legality = ULegalitySubsystem::Get(this))
		{
			return Legality->IsPlayerPerformingIllegalAction();
		}
	}

	return false;
}

void UNpcAwarenessComponent::UpdateAwareness(const float DeltaTime)
{
	const ANpcAiController* Controller = GetNpcController();
	const UNpcSensorComponent* Sensor = Controller ? Controller->GetSensorComponent() : nullptr;
	const UNpcFocusComponent* FocusComp = Controller ? Controller->GetFocusComponent() : nullptr;
	const UNpcProfile* Profile = GetNpcProfile();

	const FNpcFocusTarget& CurrentFocus = FocusComp ? FocusComp->GetCurrentFocus() : FNpcFocusTarget();
	const AActor* FocusActor = CurrentFocus.FocusActor.Get();
	const FNpcTrackedTarget* Tracked = (Sensor && FocusActor) ? Sensor->GetData().Find(FocusActor) : nullptr;
	const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;

	if (Tracked && FocusActor)
	{
		const bool bCurrentlySeen = Tracked->IsCurrentlySeen();
		const double SightAge = (Tracked->LastSeenTime >= 0.0) ? (Now - Tracked->LastSeenTime) : -1.0;
		const float GracePeriod = Profile ? Profile->LosePlayerSightGracePeriod : 1.0f;

		AwarenessState.bFocusTargetStale = (SightAge < 0.0 || SightAge > GracePeriod);
		AwarenessState.bFocusTargetVisible = bCurrentlySeen || (!AwarenessState.bFocusTargetStale && Tracked->LastSeenTime >= 0.0);

		// Distance and Always-See
		const APawn* Pawn = GetNpcPawn();
		const FVector MyLoc = Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector;
		const FVector TargetLoc = FocusActor->GetActorLocation();
		const float Dist = FVector::Dist(MyLoc, TargetLoc);
		const float AlwaysSeeRange = Profile ? Profile->AlwaysSeePlayerRange : 100.0f;
		const bool bAlwaysSee = (Dist <= AlwaysSeeRange);

		// Exposure and Crouch
		float Exposure = 1.0f;
		if (NpcAi::IsPlayerActor(FocusActor))
		{
			if (const UPlayerExposureSubsystem* ExpSubsystem = UPlayerExposureSubsystem::Get(this))
			{
				Exposure = ExpSubsystem->GetCurrentTotalExposure();
			}

			if (const ACharacter* Char = Cast<ACharacter>(FocusActor))
			{
				if (Char->bIsCrouched && Profile)
				{
					Exposure *= Profile->CrouchSuspicionMultiplier;
				}
			}
		}

		// Visibility Grade
		const float ShadowThreshold = Profile ? Profile->VisibilityExposureThreshold_Shadowed : 0.15f;
		const float FullyVisibleThreshold = Profile ? Profile->VisibilityExposureThreshold_FullyVisible : 0.6f;

		if (!bCurrentlySeen)
		{
			AwarenessState.VisibilityGrade = ENpcVisibilityGrade::Hidden;
		}
		else if (bAlwaysSee || Exposure >= FullyVisibleThreshold)
		{
			AwarenessState.VisibilityGrade = ENpcVisibilityGrade::FullyVisible;
		}
		else if (Exposure >= ShadowThreshold)
		{
			AwarenessState.VisibilityGrade = ENpcVisibilityGrade::PartiallyExposed;
		}
		else
		{
			AwarenessState.VisibilityGrade = ENpcVisibilityGrade::Shadowed;
		}

		// LOS Angle
		bool bDirectSight = false;
		if (Pawn)
		{
			const FVector Dir = (TargetLoc - MyLoc).GetSafeNormal();
			const float ForwardDot = FVector::DotProduct(Pawn->GetActorForwardVector(), Dir);
			const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(ForwardDot, -1.0f, 1.0f)));
			bDirectSight = (AngleDeg <= 45.0f);
		}

		// Suspicion Math
		const float BaseGainSight = Profile ? Profile->SuspicionGainPerSecond_Sight : 40.0f;
		const float BaseGainPeriph = Profile ? Profile->SuspicionGainPerSecond_Peripheral : 15.0f;
		const float BaseDecay = Profile ? Profile->SuspicionDecayPerSecond : 8.0f;
		const float FocusMult = FocusComp ? FocusComp->GetAwarenessMultiplier() : 1.0f;
		const bool bThreatening = IsFocusTargetThreatening(FocusActor);
		const bool bExposureGated = (Exposure >= 0.05f);
		const bool bHasEffectiveLOS = bCurrentlySeen && (bAlwaysSee || bExposureGated);

		float Delta = 0.0f;
		switch (TuningParams.Mode)
		{
		case ENpcSuspicionMode::Normal:
			if (bHasEffectiveLOS && bThreatening)
			{
				const float Rate = bDirectSight ? BaseGainSight : BaseGainPeriph;
				Delta = Rate * TuningParams.SuspicionGainMultiplier * FocusMult * Exposure * DeltaTime;
			}
			else
			{
				Delta = -BaseDecay * TuningParams.SuspicionDecayMultiplier * DeltaTime;
			}
			break;

		case ENpcSuspicionMode::FrozenWhileVisible:
			Delta = bHasEffectiveLOS ? 0.0f : -BaseDecay * TuningParams.SuspicionDecayMultiplier * DeltaTime;
			break;

		case ENpcSuspicionMode::AlwaysDecay:
			Delta = -BaseDecay * TuningParams.SuspicionDecayMultiplier * DeltaTime;
			break;
		}

		if (Delta != 0.0f)
		{
			AddSuspicion(Delta);
		}

		// Investigation Location
		if (Tracked->bHasLocation)
		{
			AwarenessState.bHasInvestigationLocation = true;
			AwarenessState.InvestigationLocation = Tracked->LastKnownLocation;
		}
		else
		{
			AwarenessState.bHasInvestigationLocation = false;
			AwarenessState.InvestigationLocation = FVector::ZeroVector;
		}
	}
	else
	{
		// No focus or unknown target => suspicion only decays
		AwarenessState.bFocusTargetVisible = false;
		AwarenessState.bFocusTargetStale = true;
		AwarenessState.VisibilityGrade = ENpcVisibilityGrade::Hidden;

		const float BaseDecay = Profile ? Profile->SuspicionDecayPerSecond : 8.0f;
		AddSuspicion(-BaseDecay * TuningParams.SuspicionDecayMultiplier * DeltaTime);

		// Investigation location uses the most recent entry of sensor data (by LastSensedTime)
		if (Sensor && Sensor->GetData().Targets.Num() > 0)
		{
			const FNpcTrackedTarget* MostRecent = nullptr;
			for (const FNpcTrackedTarget& Target : Sensor->GetData().Targets)
			{
				if (Target.bHasLocation && (!MostRecent || Target.LastSensedTime > MostRecent->LastSensedTime))
				{
					MostRecent = &Target;
				}
			}

			if (MostRecent && MostRecent->bHasLocation)
			{
				AwarenessState.bHasInvestigationLocation = true;
				AwarenessState.InvestigationLocation = MostRecent->LastKnownLocation;
			}
			else
			{
				AwarenessState.bHasInvestigationLocation = false;
				AwarenessState.InvestigationLocation = FVector::ZeroVector;
			}
		}
		else
		{
			AwarenessState.bHasInvestigationLocation = false;
			AwarenessState.InvestigationLocation = FVector::ZeroVector;
		}
	}
}
