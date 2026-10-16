#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/AI/NpcAiComponentBase.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "NpcFocusComponent.generated.h"

/**
 * Component responsible for NPC focus arbitration, physical head/body facing,
 * duration countdowns, and translating sensor events into focus requests.
 */
UCLASS(ClassGroup = "NPC|Focus", meta = (BlueprintSpawnableComponent))
class STEALTH_API UNpcFocusComponent : public UNpcAiComponentBase
{
	GENERATED_BODY()

public:
	UNpcFocusComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Focus")
	FOnNpcFocusChanged OnNpcFocusChanged;

	/**
	 * Requests a focus change.
	 * Equal priority does NOT preempt unless the current focus expired; strictly higher priority preempts.
	 * Returns true if accepted or candidate matches current focus.
	 */
	UFUNCTION(BlueprintCallable, Category = "NPC|Focus")
	bool RequestFocus(const FNpcFocusTarget& NewFocusCandidate);

	/**
	 * Clears current focus if current priority <= MaxPriorityToClear (or MaxPriorityToClear == None to clear unconditionally).
	 */
	UFUNCTION(BlueprintCallable, Category = "NPC|Focus")
	void ClearFocus(ENpcFocusPriority MaxPriorityToClear = ENpcFocusPriority::None);

	UFUNCTION(BlueprintPure, Category = "NPC|Focus")
	const FNpcFocusTarget& GetCurrentFocus() const { return CurrentFocus; }

	/** True if NPC is focusing on a specific actor */
	UFUNCTION(BlueprintPure, Category = "NPC|Focus")
	bool IsFocusedOn(const AActor* Actor) const;

	/** True if NPC is focusing on any player-controlled character */
	UFUNCTION(BlueprintPure, Category = "NPC|Focus")
	bool IsFocusedOnPlayer() const;

	/** True if NPC is focusing on a distraction (minor noise or environmental disturbance) rather than player or combat */
	UFUNCTION(BlueprintPure, Category = "NPC|Focus")
	bool IsDistracted() const;

	/** Multiplier applied to awareness/suspicion gain while focused */
	UFUNCTION(BlueprintPure, Category = "NPC|Focus")
	float GetAwarenessMultiplier() const;

	/** Applies physical focus to the AI controller */
	UFUNCTION(BlueprintCallable, Category = "NPC|Focus")
	void ApplyPhysicalFocus();

	/** Sensor event handler translating raw stimuli into focus candidates */
	void HandleSensorEvent(const FNpcSensorEvent& Event);

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|Focus")
	FNpcFocusTarget CurrentFocus;
};
