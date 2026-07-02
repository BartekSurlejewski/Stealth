#include "Characters/NPCs/Prisoners/PrisonerNpcContextComponent.h"

#include "Characters/NPCs/NpcAiController.h"
#include "Navigation/PathFollowingComponent.h"

void UPrisonerNpcContextComponent::BeginPlay()
{
	Super::BeginPlay();

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	DailyTaskStartedListenerHandle = MsgSubsystem.RegisterListener<FDailyTaskStartedMessage>(FGameplayTag::RequestGameplayTag("Message.DailyTask.OnDailyTaskStarted"), this,
	                                                                                         &UPrisonerNpcContextComponent::OnDailyTaskStarted);
	DailyTaskEndedListenerHandle = MsgSubsystem.RegisterListener<FDailyTaskEndedMessage>(FGameplayTag::RequestGameplayTag("Message.DailyTask.OnDailyTaskEnded"), this,
	                                                                                     &UPrisonerNpcContextComponent::OnDailyTaskEnded);
}

void UPrisonerNpcContextComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(DailyTaskStartedListenerHandle);
	MsgSubsystem.UnregisterListener(DailyTaskEndedListenerHandle);

	Super::EndPlay(EndPlayReason);
}


void UPrisonerNpcContextComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPrisonerNpcContextComponent::MoveToPosition(const FVector& Destination, TFunction<void(bool bSuccess)> OnComplete)
{
	if (!NpcAiController)
	{
		OnComplete(false);
		return;
	}

	PendingMoveCallback = MoveTemp(OnComplete);

	NpcAiController->ReceiveMoveCompleted.RemoveDynamic(this, &UPrisonerNpcContextComponent::HandleMoveCompleted);
	NpcAiController->ReceiveMoveCompleted.AddDynamic(this, &UPrisonerNpcContextComponent::HandleMoveCompleted);

	NpcAiController->MoveToLocation(Destination, 50.0f, true, true);
}

void UPrisonerNpcContextComponent::HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	NpcAiController->ReceiveMoveCompleted.RemoveDynamic(this, &UPrisonerNpcContextComponent::HandleMoveCompleted);

	if (PendingMoveCallback)
	{
		PendingMoveCallback(Result == EPathFollowingResult::Success);
		PendingMoveCallback = nullptr;
	}
}

void UPrisonerNpcContextComponent::OnDailyTaskStarted(FGameplayTag Channel, const FDailyTaskStartedMessage& Message)
{
	SendStateTreeEvent(Channel);
}

void UPrisonerNpcContextComponent::OnDailyTaskEnded(FGameplayTag Channel, const FDailyTaskEndedMessage& Message)
{
	SendStateTreeEvent(Channel);
}
