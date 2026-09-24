#include "Characters/NPCs/AI/Perception/NpcSensorComponent.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Exposure/PlayerExposureSubsystem.h"
#include "Legality/LegalitySubsystem.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

UNpcSensorComponent::UNpcSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UNpcSensorComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedExposureSubsystem = UPlayerExposureSubsystem::Get(this);
	CachedLegalitySubsystem = ULegalitySubsystem::Get(this);
}

void UNpcSensorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UNpcSensorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateSensoryState(DeltaTime);

	// Reset single-tick flags
	bTouchSensedThisTick = false;
	bDamageSensedThisTick = false;
}

ANpcAiController* UNpcSensorComponent::GetAiController() const
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

UNpcProfile* UNpcSensorComponent::GetNpcProfile() const
{
	if (NpcProfile)
	{
		return NpcProfile;
	}

	if (const ANpcAiController* Controller = GetAiController())
	{
		if (const UNpcContextComponent* Context = Controller->FindComponentByClass<UNpcContextComponent>())
		{
			return Context->GetNpcProfile();
		}
	}
	return nullptr;
}

float UNpcSensorComponent::GetConfiguredSightRadius() const
{
	if (const ANpcAiController* Controller = GetAiController())
	{
		if (const UAIPerceptionComponent* Perception = Controller->FindComponentByClass<UAIPerceptionComponent>())
		{
			if (const UAISenseConfig_Sight* SightConfig = Cast<UAISenseConfig_Sight>(Perception->GetSenseConfig(UAISense::GetSenseID<UAISense_Sight>())))
			{
				return SightConfig->SightRadius;
			}
		}
	}
	return 2000.0f; // Default fallback
}

void UNpcSensorComponent::OnSightStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (Actor && Actor->IsA<AStealthPlayerCharacter>())
	{
		LastPerceivedActor = Actor;
		TimeSinceLastStimulus = 0.0f;
		LastStimulusTag = Stimulus.Tag.IsNone() ? FGameplayTag::EmptyTag : FGameplayTag::RequestGameplayTag(Stimulus.Tag, false);

		if (Stimulus.WasSuccessfullySensed())
		{
			bLineOfSightValid = true;
			bLineOfSightBlocked = false;
			bWasPreviouslySensedBySight = true;
			LastValidObservationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
			LastKnownPlayerPos = Stimulus.StimulusLocation;
		}
		else
		{
			if (bWasPreviouslySensedBySight)
			{
				bLineOfSightBlocked = true;
			}
			bLineOfSightValid = false;
		}
	}
}

void UNpcSensorComponent::OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	LastPerceivedActor = Actor;
	TimeSinceLastStimulus = 0.0f;
	HearingStimulusStrength = Stimulus.Strength;
	LastHeardSoundLocation = Stimulus.StimulusLocation;
	LastHeardSoundTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	const ENpcNoiseType NoiseType = ClassifyNoiseStimulus(Stimulus);
	switch (NoiseType)
	{
	case ENpcNoiseType::Critical:
		LastStimulusTag = StealthAiTags::TAG_Noise_Critical;
		break;
	case ENpcNoiseType::Major:
		LastStimulusTag = StealthAiTags::TAG_Noise_Major;
		break;
	case ENpcNoiseType::Distraction:
		LastStimulusTag = StealthAiTags::TAG_Noise_Distraction;
		break;
	case ENpcNoiseType::Subtle:
	default:
		LastStimulusTag = StealthAiTags::TAG_Noise_Footstep;
		break;
	}

	if (ANpcAiController* Controller = GetAiController())
	{
		if (UNpcContextComponent* Context = Controller->FindComponentByClass<UNpcContextComponent>())
		{
			Context->HandleHearingStimulus(NoiseType, Stimulus);
		}
	}
}

void UNpcSensorComponent::OnPredictionStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	LastPerceivedActor = Actor;
	TimeSinceLastStimulus = 0.0f;
	LastKnownPlayerPos = Stimulus.StimulusLocation;
}

void UNpcSensorComponent::OnTouchStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	LastPerceivedActor = Actor;
	TimeSinceLastStimulus = 0.0f;
	bTouchSensedThisTick = true;
	LastStimulusTag = StealthAiTags::TAG_NPC_Event_TouchDetected;

	if (Actor && Actor->IsA<AStealthPlayerCharacter>())
	{
		bLineOfSightValid = true;
		bLineOfSightBlocked = false;
		LastValidObservationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		LastKnownPlayerPos = Stimulus.StimulusLocation;
	}

	if (ANpcAiController* Controller = GetAiController())
	{
		if (UNpcContextComponent* Context = Controller->FindComponentByClass<UNpcContextComponent>())
		{
			Context->SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_TouchDetected);
		}
	}
}

void UNpcSensorComponent::OnDamageStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	LastPerceivedActor = Actor;
	TimeSinceLastStimulus = 0.0f;
	bDamageSensedThisTick = true;
	LastStimulusTag = StealthAiTags::TAG_NPC_Event_DamageReceived;

	if (Actor && Actor->IsA<AStealthPlayerCharacter>())
	{
		bLineOfSightValid = true;
		bLineOfSightBlocked = false;
		LastValidObservationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		LastKnownPlayerPos = Stimulus.StimulusLocation;
	}

	if (ANpcAiController* Controller = GetAiController())
	{
		if (UNpcContextComponent* Context = Controller->FindComponentByClass<UNpcContextComponent>())
		{
			Context->SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_DamageReceived);
		}
	}
}

ENpcNoiseType UNpcSensorComponent::ClassifyNoiseStimulus(const FAIStimulus& Stimulus) const
{
	if (Stimulus.Tag == FName("Explosion") || Stimulus.Tag == FName("DeathScream") || Stimulus.Tag == FName("CombatNoise"))
	{
		return ENpcNoiseType::Critical;
	}

	if (Stimulus.Tag == FName("GlassShatter") || Stimulus.Tag == FName("Alarm") || Stimulus.Tag == FName("DoorForced"))
	{
		return ENpcNoiseType::Major;
	}

	if (Stimulus.Tag == FName("Distraction") || Stimulus.Tag == FName("RockThrow") || Stimulus.Tag == FName("Whistle"))
	{
		return ENpcNoiseType::Distraction;
	}

	if (Stimulus.Strength >= 0.8f)
	{
		return ENpcNoiseType::Critical;
	}
	if (Stimulus.Strength >= 0.5f)
	{
		return ENpcNoiseType::Major;
	}
	if (Stimulus.Strength >= 0.2f)
	{
		return ENpcNoiseType::Distraction;
	}

	return ENpcNoiseType::Subtle;
}

void UNpcSensorComponent::UpdateSensoryState(float DeltaTime)
{
	TimeSinceLastStimulus += DeltaTime;

	UpdateLineOfSight(DeltaTime);
	UpdatePlayerLegality();
}

void UNpcSensorComponent::UpdateLineOfSight(float DeltaTime)
{
	const ANpcAiController* Controller = GetAiController();
	if (!Controller)
	{
		return;
	}

	const APawn* ControlledPawn = Controller->GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!PlayerPawn)
	{
		bHasLineOfSight = false;
		return;
	}

	PlayerDistance = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerPawn->GetActorLocation());

	const UNpcProfile* Profile = GetNpcProfile();
	const float AlwaysSeeRange = Profile ? Profile->AlwaysSeePlayerRange : 100.0f;
	bIsFullyVisibleByDistance = (PlayerDistance <= AlwaysSeeRange);

	const float SightRadius = GetConfiguredSightRadius();
	bInDetectionRange = (PlayerDistance <= SightRadius);

	const FVector DirectionToPlayer = (PlayerPawn->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
	const float ForwardDot = FVector::DotProduct(ControlledPawn->GetActorForwardVector(), DirectionToPlayer);
	const float AngleToPlayer = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(ForwardDot, -1.0f, 1.0f)));
	bIsLookingDirectlyAtPlayer = (AngleToPlayer <= 45.0f);

	PlayerExposureMultiplier = 1.0f;
	if (CachedExposureSubsystem.IsValid())
	{
		PlayerExposureMultiplier = CachedExposureSubsystem->GetCurrentTotalExposure();
	}

	if (const ACharacter* CharacterPlayer = Cast<ACharacter>(PlayerPawn))
	{
		if (CharacterPlayer->bIsCrouched && Profile)
		{
			PlayerExposureMultiplier *= Profile->CrouchSuspicionMultiplier;
		}
	}

	const bool bExposureGated = (PlayerExposureMultiplier >= 0.05f);
	bHasLineOfSight = (bLineOfSightValid || bTouchSensedThisTick) && (bIsFullyVisibleByDistance || bExposureGated);

	if (bHasLineOfSight)
	{
		LastKnownPlayerPos = PlayerPawn->GetActorLocation();
	}
}

void UNpcSensorComponent::UpdatePlayerLegality()
{
	bIsPlayerPerformingIllegalAction = false;

	if (CachedLegalitySubsystem.IsValid())
	{
		bIsPlayerPerformingIllegalAction = CachedLegalitySubsystem->IsPlayerPerformingIllegalAction();
	}
}
