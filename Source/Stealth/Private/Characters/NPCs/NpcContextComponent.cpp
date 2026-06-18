#include "Characters/NPCs/NpcContextComponent.h"

#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Exposure/PlayerExposureSubsystem.h"
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

	GetWorld()->GetTimerManager().SetTimer(PlayerVisibilityCheckTimerHandle, this, &UNpcContextComponent::CheckPlayerVisibility, 0.25f, true);
}

void UNpcContextComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(PlayerInRestrictedAreaListenerHandle);

	GetWorld()->GetTimerManager().ClearTimer(PlayerVisibilityCheckTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(GainPlayerSightTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(LosePlayerSightTimerHandle);


	Super::EndPlay(EndPlayReason);
}

void UNpcContextComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bEffectivelySeesPlayer)
	{
		LastKnownPlayerPos = GetPlayerPawn()->GetActorLocation();
	}
}

void UNpcContextComponent::OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus)
{
	if (Actor != GetPlayerPawn())
	{
		return;
	}

	//TODO: add defining effectiveStrength threshold without hardcoding it
	bHasPlayerLineOfSight = (Stimulus.WasSuccessfullySensed() && Stimulus.Strength >= 0.05f);
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

void UNpcContextComponent::CheckPlayerVisibility()
{
	if (bHasPlayerLineOfSight)
	{
		float ExposureMultiplier = GetWorld()->GetSubsystem<UPlayerExposureSubsystem>()->GetCurrentTotalExposure();

		if (!bEffectivelySeesPlayer && ExposureMultiplier > 0.1f)
		{
			// Run timer for noticing player
			if (!GainPlayerSightTimerHandle.IsValid())
			{
				GetWorld()->GetTimerManager().ClearTimer(LosePlayerSightTimerHandle);
				GetWorld()->GetTimerManager().SetTimer(GainPlayerSightTimerHandle, this, &UNpcContextComponent::GainPlayerSight, Profile->GainPlayerSightGracePeriod,
				                                       false);
			}
		}
		else if (ExposureMultiplier < 0.1f)
		{
			// Run timer for losing player sight
			if (!LosePlayerSightTimerHandle.IsValid())
			{
				GetWorld()->GetTimerManager().ClearTimer(GainPlayerSightTimerHandle);
				GetWorld()->GetTimerManager().SetTimer(LosePlayerSightTimerHandle, this, &UNpcContextComponent::LosePlayerSight, Profile->LosePlayerSightGracePeriod,
				                                       false);
			}
		}
	}
	else if (bEffectivelySeesPlayer)
	{
		// Run timer for losing player sight
		if (!LosePlayerSightTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(GainPlayerSightTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(LosePlayerSightTimerHandle, this, &UNpcContextComponent::LosePlayerSight, Profile->LosePlayerSightGracePeriod,
			                                       false);
		}
	}
}


void UNpcContextComponent::GainPlayerSight()
{
	bEffectivelySeesPlayer = true;

	if (bIsPlayerInRestrictedArea)
	{
		SendStateTreeEvent(SuspiciousActivityTag);
	}

	OnPlayerInSightChanged.Broadcast(bEffectivelySeesPlayer);
}

void UNpcContextComponent::LosePlayerSight()
{
	bEffectivelySeesPlayer = false;
	OnPlayerInSightChanged.Broadcast(bEffectivelySeesPlayer);
}

void UNpcContextComponent::OnPlayerInRestrictedAreaChanged(FGameplayTag Channel, const FBooleanMessage& Message)
{
	bIsPlayerInRestrictedArea = Message.bValue;

	if (!Message.bValue || !bEffectivelySeesPlayer)
	{
		// Guard doesn't see player, do nothing
		return;
	}

	SendStateTreeEvent(SuspiciousActivityTag);
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
