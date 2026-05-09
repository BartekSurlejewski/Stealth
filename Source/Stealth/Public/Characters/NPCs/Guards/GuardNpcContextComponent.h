#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "GuardNpcContextComponent.generated.h"


class UStateTreeComponent;
class UNpcPatrolComponent;
struct FAIStimulus;
class UGuardNpcProfile;

UENUM(BlueprintType)
enum class EGuardAlertLevel : uint8
{
	Patrol, Suspicious, Alerted, Search, Alarm
};

UCLASS(ClassGroup="NPC", meta=(BlueprintSpawnableComponent))
class STEALTH_API UGuardNpcContextComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGuardNpcContextComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Guard")
	TObjectPtr<UGuardNpcProfile> Profile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Guard")
	TArray<TObjectPtr<AActor>> PatrolPoints;

	// State (read from State Tree)
	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	EGuardAlertLevel AlertLevel = EGuardAlertLevel::Patrol;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	float SuspicionLevel = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	FVector LastKnownPlayerPos = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	FVector LastHeardSoundLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	bool bPlayerInDirectSight = false;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	bool bPlayerInPeripheralSight = false;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	float SearchTimer = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	int32 GlobalAlarmLevel = 0;

	// Patrol
	UPROPERTY(BlueprintReadWrite, Category="Guard|Patrol")
	int32 CurrentPatrolIndex = 0;

	UFUNCTION(BlueprintPure)
	AActor* GetCurrentPatrolPoint() const;

	// External systems API
	UFUNCTION(BlueprintCallable)
	void ForceAlert(FVector AtLocation);

	UFUNCTION(BlueprintCallable)
	void BeginSearch();

	UFUNCTION(BlueprintPure)
	bool IsSearchExpired() const { return SearchTimer <= 0.f; }

	// Perception callbacks
	void OnSightStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	void OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);

private:
	UPROPERTY()
	TSoftObjectPtr<UNpcPatrolComponent> PatrolComponent;
	UPROPERTY()
	TSoftObjectPtr<UStateTreeComponent> StateTreeComponent;

	UPROPERTY(EditDefaultsOnly, Category="Guard|State|Tags")
	FGameplayTag SearchExpiredTag;
	UPROPERTY(EditDefaultsOnly, Category="Guard|State|Tags")
	FGameplayTag AlertThresholdMetTag;
	UPROPERTY(EditDefaultsOnly, Category="Guard|State|Tags")
	FGameplayTag GlobalAlarmTag;

	void TickSuspicion(float DeltaTime);
	void UpdateAlertLevel();
	void SendGuardEvent(FGameplayTag Tag) const;
	void OnAlarmChanged(int32 NewLevel, const FVector& SourceLocation);
	float GetSuspicionModifier(AActor* Target) const;
};
