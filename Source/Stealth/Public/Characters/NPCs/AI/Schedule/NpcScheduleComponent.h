#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Characters/NPCs/AI/Schedule/NpcScheduleAsset.h"
#include "NpcScheduleComponent.generated.h"

struct FTimeChangedMessage;
class UActivityPointSubsystem;
class UNpcContextComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScheduleSlotChanged, const FNpcScheduleSlot&, NewSlot, const FNpcScheduleSlot&, PreviousSlot);

UCLASS(ClassGroup = (NPC), meta = (BlueprintSpawnableComponent))
class STEALTH_API UNpcScheduleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcScheduleComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Assign or override schedule asset at runtime */
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void SetScheduleAsset(UNpcScheduleAsset* InScheduleAsset);

	/** Force evaluation of the schedule for current or specific time */
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void EvaluateSchedule(int32 WorldTimeInMinutes = -1);

	UFUNCTION(BlueprintPure, Category = "Schedule")
	const FNpcScheduleSlot& GetActiveSlot() const { return ActiveSlot; }

	UFUNCTION(BlueprintPure, Category = "Schedule")
	AActor* GetClaimedActivityPoint() const { return ClaimedActivityPoint.Get(); }

	UFUNCTION(BlueprintPure, Category = "Schedule")
	FGameplayTag GetActiveActivityTag() const { return ActiveSlot.ActivityTag; }

	UFUNCTION(BlueprintPure, Category = "Schedule")
	FGameplayTag GetActiveLocationTag() const { return ActiveSlot.LocationTag; }

public:
	UPROPERTY(BlueprintAssignable, Category = "Schedule|Events")
	FOnScheduleSlotChanged OnScheduleSlotChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	TObjectPtr<UNpcScheduleAsset> ScheduleAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Schedule|Tags")
	FGameplayTag ScheduleActivityChangedEventTag;

	UFUNCTION(BlueprintPure, Category = "Schedule")
	UNpcContextComponent* GetContextComponent() const;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Schedule")
	FNpcScheduleSlot ActiveSlot;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Schedule")
	TWeakObjectPtr<AActor> ClaimedActivityPoint;

	FGuid GetOwnerNpcGuid() const;
	void ClaimCurrentActivityPoint();
	void ReleaseCurrentActivityPoint();

private:
	void OnWorldTimeChanged(FGameplayTag Channel, const FTimeChangedMessage& Message);

	FGameplayMessageListenerHandle TimeChangedListenerHandle;
	bool bHasActiveSlot = false;
};
