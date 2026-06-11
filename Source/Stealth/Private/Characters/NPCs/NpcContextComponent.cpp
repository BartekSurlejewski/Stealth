#include "Characters/NPCs/NpcContextComponent.h"

#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Messages/StealthMessages.h"
#include "Perception/AIPerceptionTypes.h"


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

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	PlayerInRestrictedAreaListenerHandle = MsgSubsystem.RegisterListener<FBooleanMessage>(FGameplayTag::RequestGameplayTag("Message.Player.IsInRestrictedAreaChanged"), this,
	                                                                                      &UNpcContextComponent::OnPlayerInRestrictedAreaChanged);
}

void UNpcContextComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(PlayerInRestrictedAreaListenerHandle);

	Super::EndPlay(EndPlayReason);
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

	LastHeardSoundLocation = Stimulus.StimulusLocation;
}

bool UNpcContextComponent::IsPlayerInRestrictedArea() { return bIsPlayerInRestrictedArea; }

void UNpcContextComponent::OnPlayerInRestrictedAreaChanged(FGameplayTag Channel, const FBooleanMessage& Message)
{
	bIsPlayerInRestrictedArea = Message.bValue;
}

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
