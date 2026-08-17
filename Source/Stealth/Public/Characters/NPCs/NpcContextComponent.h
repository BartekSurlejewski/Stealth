#pragma once

#include "CoreMinimal.h"
#include "Characters/Player/StealthPlayerState.h"
#include "Components/ActorComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "NpcContextComponent.generated.h"


class UPlayerExposureSubsystem;
struct FBooleanMessage;
struct FAIStimulus;
class UNpcProfile;
class ANpcAiController;

UENUM(BlueprintType)
enum class EGuardBehaviourState : uint8
{
	Patrol, Suspicious, Alerted, Search, Alarm
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBehaviourStateChanged, EGuardBehaviourState, NewBehaviourState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerInSightChanged, bool, IsPlayerInDirectSight);


UCLASS(Abstract, Blueprintable, ClassGroup="NPC")
class STEALTH_API UNpcContextComponent : public UActorComponent
{
	GENERATED_BODY()
	/*Events*/
public:
	UPROPERTY(BlueprintAssignable)
	FOnBehaviourStateChanged OnBehaviourStateChanged;
	UPROPERTY(BlueprintAssignable)
	FOnPlayerInSightChanged OnPlayerInSightChanged;

	/*Methods*/
public:
	UNpcContextComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Perception callbacks
	UFUNCTION()
	virtual void OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus);
	UFUNCTION()
	virtual void OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);
	UFUNCTION(BlueprintPure)
	bool IsPlayerInRestrictedArea();
	UFUNCTION()
	void SendStateTreeEvent(const FGameplayTag& Tag) const;

protected:
	UFUNCTION()
	void CheckPlayerVisibility();
	UFUNCTION()
	void GainPlayerSight();
	UFUNCTION()
	void LosePlayerSight();

	UFUNCTION()
	AStealthPlayerCharacter* GetPlayerCharacter() const;

private:
	void OnPlayerInRestrictedAreaChanged(FGameplayTag Channel, const FBooleanMessage& Message);

	/*Properties*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	TObjectPtr<UNpcProfile> Profile;
	UPROPERTY(BlueprintReadOnly, Category="NPC|State")
	FVector LastKnownPlayerPos = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category="NPC|State")
	FVector LastHeardSoundLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category="NPC|State")
	bool bHasPlayerLineOfSight = false;
	UPROPERTY(BlueprintReadOnly, Category="NPC|State")
	bool bEffectivelySeesPlayer = false;
	UPROPERTY(EditDefaultsOnly, Category="NPC|State|Tags")
	FGameplayTag SuspiciousActivityTag;
	UPROPERTY()
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;
	UPROPERTY()
	TObjectPtr<ANpcAiController> NpcAiController;
	UPROPERTY()
	FTimerHandle PlayerVisibilityCheckTimerHandle;
	UPROPERTY()
	FTimerHandle GainPlayerSightTimerHandle;
	UPROPERTY()
	FTimerHandle LosePlayerSightTimerHandle;

private:
	UPROPERTY()
	bool bIsPlayerInRestrictedArea;
	FGameplayMessageListenerHandle PlayerInRestrictedAreaListenerHandle;
};
