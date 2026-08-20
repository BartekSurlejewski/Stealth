#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "NpcFocusComponent.generated.h"

class ANpcAiController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNpcFocusChanged, const FNpcFocusTarget&, NewFocus, const FNpcFocusTarget&, PreviousFocus);

/**
 * Component responsible exclusively for NPC focus arbitration, physical head/body facing,
 * focus duration countdowns, and distraction awareness calculation.
 */
UCLASS(ClassGroup = "NPC|Focus", meta = (BlueprintSpawnableComponent))
class STEALTH_API UNpcFocusComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcFocusComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Focus")
	FOnNpcFocusChanged OnNpcFocusChanged;

	/** Requests a focus change. Returns true if accepted (higher/equal priority or expired) */
	UFUNCTION(BlueprintCallable, Category = "NPC|Focus")
	bool RequestFocus(const FNpcFocusTarget& NewFocusCandidate);

	/** Clears the current focus if the matching priority or lower is requested (or None to clear unconditionally) */
	UFUNCTION(BlueprintCallable, Category = "NPC|Focus")
	void ClearFocus(ENpcFocusPriority MinimumPriorityToClear = ENpcFocusPriority::None);

	UFUNCTION(BlueprintPure, Category = "NPC|Focus")
	const FNpcFocusTarget& GetCurrentFocus() const { return CurrentFocus; }

	/** True if NPC is focusing on a distraction (minor noise or environmental disturbance) rather than player or combat */
	UFUNCTION(BlueprintPure, Category = "NPC|Focus")
	bool IsDistracted() const;

	/** Multiplier applied to awareness/suspicion gain while distracted */
	UFUNCTION(BlueprintPure, Category = "NPC|Focus")
	float GetAwarenessMultiplier() const;

	/** Applies physical focus to the AI controller */
	UFUNCTION(BlueprintCallable, Category = "NPC|Focus")
	void ApplyPhysicalFocus();

	UFUNCTION(BlueprintPure, Category = "NPC|Focus")
	ANpcAiController* GetAiController() const;

protected:
	void UpdateFocus(float DeltaTime);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "NPC|Focus")
	FNpcFocusTarget CurrentFocus;

	UPROPERTY()
	mutable TWeakObjectPtr<ANpcAiController> CachedController;
};
