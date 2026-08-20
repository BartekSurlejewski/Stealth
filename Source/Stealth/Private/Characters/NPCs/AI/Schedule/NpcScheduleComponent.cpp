#include "Characters/NPCs/AI/Schedule/NpcScheduleComponent.h"
#include "Characters/NPCs/AI/ActivityPointSubsystem.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcCharacter.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "TimeSystem/TimeSubsystem.h"
#include "Messages/StealthMessages.h"

UNpcScheduleComponent::UNpcScheduleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	ScheduleActivityChangedEventTag = StealthAiTags::TAG_NPC_Event_ResumeRoutine;
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

			if (UNpcContextComponent* Context = GetContextComponent())
			{
				if (ScheduleActivityChangedEventTag.IsValid())
				{
					Context->SendStateTreeEvent(ScheduleActivityChangedEventTag);
				}
				Context->SendStateTreeEvent(StealthAiTags::TAG_NPC_Event_ResumeRoutine);
			}
		}
	}
}

UNpcContextComponent* UNpcScheduleComponent::GetContextComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		if (UNpcContextComponent* Context = Owner->FindComponentByClass<UNpcContextComponent>())
		{
			return Context;
		}
		if (const APawn* OwnerPawn = Cast<APawn>(Owner))
		{
			if (const AController* Controller = OwnerPawn->GetController())
			{
				if (UNpcContextComponent* CtrlContext = Controller->FindComponentByClass<UNpcContextComponent>())
				{
					return CtrlContext;
				}
			}
		}
		else if (const AController* Controller = Cast<AController>(Owner))
		{
			if (const APawn* PossessedPawn = Controller->GetPawn())
			{
				if (UNpcContextComponent* PawnContext = PossessedPawn->FindComponentByClass<UNpcContextComponent>())
				{
					return PawnContext;
				}
			}
		}
	}
	return nullptr;
}

FGuid UNpcScheduleComponent::GetOwnerNpcGuid() const
{
	if (const ANpcCharacter* Npc = Cast<ANpcCharacter>(GetOwner()))
	{
		return Npc->GetNpcGUID();
	}

	if (const APawn* Pawn = Cast<APawn>(GetOwner()))
	{
		if (const ANpcCharacter* PawnNpc = Cast<ANpcCharacter>(Pawn))
		{
			return PawnNpc->GetNpcGUID();
		}
	}

	if (const ANpcAiController* Controller = Cast<ANpcAiController>(GetOwner()))
	{
		if (const ANpcCharacter* PossessedNpc = Cast<ANpcCharacter>(Controller->GetPawn()))
		{
			return PossessedNpc->GetNpcGUID();
		}

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
