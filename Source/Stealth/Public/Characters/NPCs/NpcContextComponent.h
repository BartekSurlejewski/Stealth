#pragma once

#include "CoreMinimal.h"
#include "Characters/Player/StealthPlayerState.h"
#include "Components/ActorComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "NpcContextComponent.generated.h"


class UNpcProfile;
class ANpcAiController;

UENUM(BlueprintType)
enum class EGuardBehaviourState : uint8
{
	Patrol, Suspicious, Alerted, Search, Alarm
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBehaviourStateChanged, EGuardBehaviourState, NewBehaviourState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerInSightChanged, bool, IsPlayerInDirectSight);


UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UNpcContextComponent : public UActorComponent
{
	GENERATED_BODY()
	/*Events*/
public:
	UPROPERTY(BlueprintAssignable)
	FOnBehaviourStateChanged OnBehaviourStateChanged;
	UPROPERTY(BlueprintAssignable)
	FOnPlayerInSightChanged OnPlayerInSightChanged;

public:
	UNpcContextComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Perception callbacks
	UFUNCTION()
	virtual void OnSightStimulus(const AActor* Actor, const FAIStimulus& Stimulus, float ExposureMultiplier);
	UFUNCTION()
	virtual void OnHearingStimulus(AActor* Actor, const FAIStimulus& Stimulus);

	UFUNCTION(BlueprintPure)
	bool IsPlayerInRestrictedArea();

	UPROPERTY(BlueprintReadOnly, Category="NPC|State")
	FVector LastKnownPlayerPos = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="NPC|State")
	FVector LastHeardSoundLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="NPC|State")
	bool bPlayerInSight = false;

	UPROPERTY(BlueprintReadOnly, Category="NPC|State")
	bool bIsWaitingToLosePlayerSight = false;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NPC")
	TObjectPtr<UNpcProfile> Profile;
	UPROPERTY(EditDefaultsOnly, Category="NPC|State|Tags")
	FGameplayTag SuspiciousActivityTag;
	UPROPERTY()
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;
	UPROPERTY()
	TObjectPtr<ANpcAiController> NpcAiController;
	UPROPERTY()
	TSoftObjectPtr<APawn> PlayerPawn;
	UPROPERTY()
	TSoftObjectPtr<AStealthPlayerState> PlayerState;
	UFUNCTION()
	void SendStateTreeEvent(const FGameplayTag& Tag) const;

	//TODO: think of moving to some subsystem not to hold reference for each NPC
	APawn* GetPlayerPawn();
	AStealthPlayerState* GetPlayerState();

private:
	UPROPERTY()
	float LosePlayerSightTimer = 0.0f;
};
