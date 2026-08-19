#include "Characters/NPCs/AI/Schedule/NpcScheduleComponent.h"
#include "Characters/NPCs/AI/ActivityPointSubsystem.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcCharacter.h"
#include "TimeSystem/TimeSubsystem.h"
#include "Messages/StealthMessages.h"

UNpcScheduleComponent::UNpcScheduleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNpcScheduleComponent::BeginPlay()
{
	Super::BeginPlay();

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	TimeChangedListenerHandle = MessageSubsystem.RegisterListener<FTimeChangedMessage>(
		StealthMessageChannels::TAG_Message_Time_TimeChanged,
		this,
		&UNpcScheduleComponent::OnWorldTimeChanged
	);

	if (const UTimeSubsystem* TimeSubsystem = UTimeSubsystem::Get(this))
	{
		EvaluateSchedule(TimeSubsystem->GetCurrentDayTimeAsMinutes());
	}
	else
	{
		EvaluateSchedule(0);
	}
}

void UNpcScheduleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ReleaseCurrentActivityPoint();

	if (TimeChangedListenerHandle.IsValid())
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
		MessageSubsystem.UnregisterListener(TimeChangedListenerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UNpcScheduleComponent::SetScheduleAsset(UNpcScheduleAsset* InScheduleAsset)
{
	ScheduleAsset = InScheduleAsset;
	if (const UTimeSubsystem* TimeSubsystem = UTimeSubsystem::Get(this))
	{
		EvaluateSchedule(TimeSubsystem->GetCurrentDayTimeAsMinutes());
	}
	else
	{
		EvaluateSchedule(0);
	}
}

void UNpcScheduleComponent::EvaluateSchedule(int32 WorldTimeInMinutes)
{
	if (!ScheduleAsset)
	{
		return;
	}

	if (WorldTimeInMinutes < 0)
	{
		if (const UTimeSubsystem* TimeSubsystem = UTimeSubsystem::Get(this))
		{
			WorldTimeInMinutes = TimeSubsystem->GetCurrentDayTimeAsMinutes();
		}
		else
		{
			WorldTimeInMinutes = 0;
		}
	}

	FNpcScheduleSlot MatchedSlot;
	if (ScheduleAsset->GetSlotForTime(WorldTimeInMinutes, MatchedSlot))
	{
		const bool bSlotChanged = !bHasActiveSlot || (MatchedSlot.ActivityTag != ActiveSlot.ActivityTag) || (MatchedSlot.StartHour != ActiveSlot.StartHour);
		if (bSlotChanged)
		{
			FNpcScheduleSlot PreviousSlot = ActiveSlot;
			ActiveSlot = MatchedSlot;
			bHasActiveSlot = true;

			ClaimCurrentActivityPoint();

			OnScheduleSlotChanged.Broadcast(ActiveSlot, PreviousSlot);

			if (ScheduleActivityChangedEventTag.IsValid())
			{
				if (UNpcContextComponent* Context = GetOwner()->FindComponentByClass<UNpcContextComponent>())
				{
					Context->SendStateTreeEvent(ScheduleActivityChangedEventTag);
				}
			}
		}
	}
}

FGuid UNpcScheduleComponent::GetOwnerNpcGuid() const
{
	if (const ANpcCharacter* Npc = Cast<ANpcCharacter>(GetOwner()))
	{
		return Npc->GetNpcGUID();
	}

	if (const ANpcAiController* Controller = Cast<ANpcAiController>(GetOwner()))
	{
		if (const UCharactersRegistrySubsystem* Registry = UCharactersRegistrySubsystem::Get(this))
		{
			return Registry->GetNpcGuidByController(const_cast<ANpcAiController*>(Controller));
		}
	}

	return FGuid();
}

void UNpcScheduleComponent::ClaimCurrentActivityPoint()
{
	ReleaseCurrentActivityPoint();

	if (!ActiveSlot.ActivityTag.IsValid())
	{
		return;
	}

	UActivityPointSubsystem* ActivitySubsystem = UActivityPointSubsystem::Get(this);
	if (!ActivitySubsystem)
	{
		return;
	}

	const FGuid OwnerGuid = GetOwnerNpcGuid();
	const FVector RequesterLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

	AActor* Claimed = ActivitySubsystem->ClaimActivityPoint(OwnerGuid, ActiveSlot.ActivityTag, RequesterLocation);
	ClaimedActivityPoint = Claimed;
}

void UNpcScheduleComponent::ReleaseCurrentActivityPoint()
{
	if (ClaimedActivityPoint.IsValid())
	{
		if (UActivityPointSubsystem* ActivitySubsystem = UActivityPointSubsystem::Get(this))
		{
			ActivitySubsystem->ReleaseActivityPoint(GetOwnerNpcGuid(), ClaimedActivityPoint.Get());
		}
		ClaimedActivityPoint = nullptr;
	}
}

void UNpcScheduleComponent::OnWorldTimeChanged(FGameplayTag Channel, const FTimeChangedMessage& Message)
{
	const int32 TimeInMinutes = Message.NewHour * 60 + Message.NewMinute;
	EvaluateSchedule(TimeInMinutes);
}
