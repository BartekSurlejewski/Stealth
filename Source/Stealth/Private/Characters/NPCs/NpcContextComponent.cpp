#include "Characters/NPCs/NpcContextComponent.h"

#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionTypes.h"
#include "Stealth/Stealth.h"


UNpcContextComponent::UNpcContextComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz
}


void UNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();

	StateTreeComponent = GetOwner()->FindComponentByClass<UStateTreeAIComponent>();
	NpcAiController = Cast<ANpcAiController>(GetOwner());
}

void UNpcContextComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bPlayerInSight)
	{
		LastKnownPlayerPos = GetPlayerPawn()->GetActorLocation();
	}

	if (bIsWaitingToLosePlayerSight)
	{
		LosePlayerSightTimer -= DeltaTime;
		if (LosePlayerSightTimer <= 0.0f)
		{
			bPlayerInSight = false;
			bIsWaitingToLosePlayerSight = false;
		}
	}
}

void UNpcContextComponent::OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus, float ExposureMultiplier)
{
	if (Actor != GetPlayerPawn())
	{
		return;
	}

	const float EffectiveStrength = Stimulus.Strength * ExposureMultiplier;
	//TODO: add defining effectiveStrength threshold without hardcoding it
	if (Stimulus.WasSuccessfullySensed() && EffectiveStrength >= 0.05f)
	{
		bIsWaitingToLosePlayerSight = false;
		LosePlayerSightTimer = 0.0f;

		// LastKnownPlayerPos = Stimulus.StimulusLocation;
		bPlayerInSight = true;
	}
	else
	{
		bIsWaitingToLosePlayerSight = true;
		LosePlayerSightTimer = Profile ? Profile->LosePlayerSightGracePeriod : 2.0f;
	}

	if (bPlayerInSight && IsPlayerInRestrictedArea())
	{
		SendStateTreeEvent(SuspiciousActivityTag);
	}

	OnPlayerInSightChanged.Broadcast(bPlayerInSight);
}

void UNpcContextComponent::OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed() || Actor != GetPlayerPawn())
	{
		return;
	}

	UE_LOG(LogStealth, Log, TEXT("Heard player at %s"), *Stimulus.StimulusLocation.ToString());
	LastHeardSoundLocation = Stimulus.StimulusLocation;
}

bool UNpcContextComponent::IsPlayerInRestrictedArea() { return GetPlayerState()->GetIsInRestrictedArea(); }

void UNpcContextComponent::SendStateTreeEvent(const FGameplayTag& Tag) const
{
	if (!StateTreeComponent)
	{
		return;
	}
	StateTreeComponent->SendStateTreeEvent(FStateTreeEvent(Tag));
}

APawn* UNpcContextComponent::GetPlayerPawn()
{
	if (PlayerPawn == nullptr)
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	}

	return PlayerPawn.Get();
}

AStealthPlayerState* UNpcContextComponent::GetPlayerState()
{
	if (PlayerState == nullptr)
	{
		PlayerState = GetPlayerPawn()->GetPlayerState<AStealthPlayerState>();
	}

	return PlayerState.Get();
}
