#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "NpcAwarenessComponent.generated.h"

class UNpcSensorComponent;
class UNpcFocusComponent;
class UNpcContextComponent;
class UNpcProfile;
class ANpcAiController;

/**
 * Awareness component responsible for consuming raw sensory facts from UNpcSensorComponent
 * and computing suspicion, graded visibility, memory staleness, and investigation targets.
 * Owns the suspicion math and writes the FStateTreeAiPerceptionData snapshot.
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class STEALTH_API UNpcAwarenessComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcAwarenessComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Tuning Setter called by FStateTreeTask_ApplyPerceptionProfile
	UFUNCTION(BlueprintCallable, Category = "AI|Perception|Awareness")
	void SetPerceptionTuning(const FNpcPerceptionTuningParams& InParams);

	UFUNCTION(BlueprintCallable, Category = "AI|Perception|Awareness")
	void SetPerceptionTuningDirect(ENpcSuspicionMode InMode, float InGainMultiplier, float InDecayMultiplier);

	// Suspicion API
	UFUNCTION(BlueprintPure, Category = "AI|Perception|Awareness")
	float GetSuspicion() const { return CurrentSuspicion; }

	UFUNCTION(BlueprintCallable, Category = "AI|Perception|Awareness")
	void SetSuspicion(float NewSuspicion);

	UFUNCTION(BlueprintCallable, Category = "AI|Perception|Awareness")
	void AddSuspicion(float Delta);

	// Getters
	UFUNCTION(BlueprintPure, Category = "AI|Perception|Awareness")
	ENpcVisibilityGrade GetVisibilityGrade() const { return VisibilityGrade; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Awareness")
	float GetLostPlayerSightDuration() const { return LostPlayerSightDuration; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Awareness")
	bool IsMemoryStale() const { return bMemoryStale; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Awareness")
	FVector GetInvestigationTarget() const { return InvestigationTarget; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Awareness")
	bool IsPlayerVisible() const { return bIsPlayerVisible; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Awareness")
	const FStateTreeAiPerceptionData& GetPerceptionSnapshot() const { return PerceptionSnapshot; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Awareness")
	const FNpcPerceptionTuningParams& GetTuningParams() const { return TuningParams; }

	UFUNCTION(BlueprintPure, Category = "AI|Perception|Awareness")
	UNpcProfile* GetNpcProfile() const;

	UPROPERTY(BlueprintAssignable, Category = "AI|Perception|Awareness")
	FOnSuspicionChanged OnSuspicionChanged;

protected:
	void UpdateAwareness(float DeltaTime);
	void UpdateSuspicion(float DeltaTime);
	void UpdateVisibilityGrade();
	void UpdateInvestigationTarget();
	void BuildPerceptionSnapshot();
	void DrawDebugVisuals();

	UNpcSensorComponent* GetSensorComponent() const;
	UNpcFocusComponent* GetFocusComponent() const;
	UNpcContextComponent* GetContextComponent() const;
	ANpcAiController* GetAiController() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Awareness")
	float CurrentSuspicion = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Awareness")
	FNpcPerceptionTuningParams TuningParams;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Awareness")
	ENpcVisibilityGrade VisibilityGrade = ENpcVisibilityGrade::Hidden;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Awareness")
	bool bIsPlayerVisible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Awareness")
	float LostPlayerSightDuration = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Awareness")
	bool bMemoryStale = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Awareness")
	FVector InvestigationTarget = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Perception|Awareness")
	FStateTreeAiPerceptionData PerceptionSnapshot;

private:
	mutable TWeakObjectPtr<UNpcSensorComponent> CachedSensorComponent;
	mutable TWeakObjectPtr<UNpcFocusComponent> CachedFocusComponent;
	mutable TWeakObjectPtr<UNpcContextComponent> CachedContextComponent;
	mutable TWeakObjectPtr<ANpcAiController> CachedController;
};
