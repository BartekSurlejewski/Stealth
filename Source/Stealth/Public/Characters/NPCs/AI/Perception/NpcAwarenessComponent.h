#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/AI/NpcAiComponentBase.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "NpcAwarenessComponent.generated.h"

class UNpcSensorComponent;
class UNpcFocusComponent;
class UNpcContextComponent;
class UNpcProfile;
class ULegalitySubsystem;
class UPlayerExposureSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSuspicionThresholdReached, float, CurrentSuspicion);

/**
 * Awareness component responsible for consuming sensory facts from UNpcSensorComponent
 * for the focused target and computing suspicion, graded visibility, and investigation targets.
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class STEALTH_API UNpcAwarenessComponent : public UNpcAiComponentBase
{
	GENERATED_BODY()

public:
	UNpcAwarenessComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Tuning Setter
	UFUNCTION(BlueprintCallable, Category = "AI|Perception|Awareness")
	void SetPerceptionTuning(const FNpcPerceptionTuningParams& InParams);

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Awareness")
	const FNpcPerceptionTuningParams& GetTuningParams() const { return TuningParams; }

	// Suspicion API
	UFUNCTION(BlueprintPure, Category = "AI|Perception|Awareness")
	float GetSuspicion() const { return AwarenessState.Suspicion; }

	UFUNCTION(BlueprintCallable, Category = "AI|Perception|Awareness")
	void SetSuspicion(float NewSuspicion);

	UFUNCTION(BlueprintCallable, Category = "AI|Perception|Awareness")
	void AddSuspicion(float Delta);

	// Output State Access
	UFUNCTION(BlueprintPure, Category = "AI|Perception|Awareness")
	const FNpcAwarenessState& GetAwarenessState() const { return AwarenessState; }

	// Instant Sensor Event Handler
	void HandleSensorEvent(const FNpcSensorEvent& Event);

	/** Single extension seam for checking if focus target warrants suspicion gain */
	virtual bool IsFocusTargetThreatening(const AActor* TargetActor) const;

	UPROPERTY(BlueprintAssignable, Category = "AI|Perception|Awareness")
	FOnSuspicionThresholdReached OnSuspiciousThresholdCrossed;

protected:
	void UpdateAwareness(float DeltaTime);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Awareness")
	FNpcPerceptionTuningParams TuningParams;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Awareness")
	FNpcAwarenessState AwarenessState;

	bool bWasAboveSuspiciousThreshold = false;
};
