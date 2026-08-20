#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Messages/StealthMessages.h"
#include "PrisonerNpcContextComponent.generated.h"

UCLASS()
class STEALTH_API UPrisonerNpcContextComponent : public UNpcContextComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void MoveToPosition(const FVector& Destination, TFunction<void(bool bSuccess)> OnComplete);

private:
	UFUNCTION()
	void HandleMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
	UFUNCTION()
	void OnDailyTaskStarted(FGameplayTag Channel, const FDailyTaskStartedMessage& Message);
	UFUNCTION()
	void OnDailyTaskEnded(FGameplayTag Channel, const FDailyTaskEndedMessage& Message);

	TFunction<void(bool bSuccess)> PendingMoveCallback;

	FGameplayMessageListenerHandle DailyTaskStartedListenerHandle;
	FGameplayMessageListenerHandle DailyTaskEndedListenerHandle;
};
