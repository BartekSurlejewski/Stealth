#include "Characters/NPCs/NpcContextComponent.h"

#include "TimerManager.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Engine/World.h"
#include "Exposure/PlayerExposureSubsystem.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Messages/StealthMessages.h"
#include "Perception/AIPerceptionTypes.h"
#include "Stealth/Stealth.h"


UNpcContextComponent::UNpcContextComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz
}


void UNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(true);

	StateTreeComponent = GetOwner()->FindComponentByClass<UStateTreeAIComponent>();
	NpcAiController = Cast<ANpcAiController>(GetOwner());

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	PlayerInRestrictedAreaListenerHandle = MsgSubsystem.RegisterListener<FBooleanMessage>(StealthMessageChannels::TAG_Message_Player_IsInRestrictedAreaChanged, this,
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
		const AStealthPlayerCharacter* PlayerCharacter = GetPlayerCharacter();
		LastKnownPlayerPos = PlayerCharacter->GetActorLocation();
	}
}

void UNpcContextComponent::OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus)
{
	if (Actor != GetPlayerCharacter())
	{
		return;
	}

	//TODO: add defining effectiveStrength threshold without hardcoding it
	bHasPlayerLineOfSight = (Stimulus.WasSuccessfullySensed() && Stimulus.Strength >= 0.05f);
}

void UNpcContextComponent::OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Stimulus.WasSuccessfullySensed() || Actor != GetPlayerCharacter())
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
		float ExposureMultiplier = 1;

		if (auto ExposureSubsystem = UPlayerExposureSubsystem::Get(this))
		{
			ExposureMultiplier = ExposureSubsystem->GetCurrentTotalExposure();
		}
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
	if (bEffectivelySeesPlayer)
	{
		return;
	}

	bEffectivelySeesPlayer = true;

	if (bIsPlayerInRestrictedArea)
	{
		SendStateTreeEvent(SuspiciousActivityTag);
	}

	OnPlayerInSightChanged.Broadcast(bEffectivelySeesPlayer);
}

void UNpcContextComponent::LosePlayerSight()
{
	if (!bEffectivelySeesPlayer)
	{
		return;
	}

	bEffectivelySeesPlayer = false;
	OnPlayerInSightChanged.Broadcast(bEffectivelySeesPlayer);
}

AStealthPlayerCharacter* UNpcContextComponent::GetPlayerCharacter() const
{
	UCharactersRegistrySubsystem* CharactersRegistry = UCharactersRegistrySubsystem::Get(this);
	if (!CharactersRegistry)
	{
		UE_LOG(LogStealth, Error, TEXT("UNpcContextComponent::GetPlayerCharacter: CharactersRegistry is null"));
		return nullptr;
	}

	return CharactersRegistry->GetPlayerCharacter();
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
