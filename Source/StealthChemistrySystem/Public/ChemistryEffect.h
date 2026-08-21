#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "ChemistryTypes.h"
#include "ChemistryEffect.generated.h"

class AActor;

/**
 * Base class for all modular chemistry effect handlers (e.g. destroy, ignite, attract attention, impulse).
 * Instances are cached and executed by the UChemistrySubsystem.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class STEALTHCHEMISTRYSYSTEM_API UChemistryEffect : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;

	/**
	 * Executes the effect.
	 *
	 * @param EffectTag The tag of the effect being executed.
	 * @param Context The element application context that triggered this effect.
	 * @param AffectedActor The specific actor this effect targets (TargetActor, Instigator/Emitter, or nullptr if location-based).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chemistry")
	void ExecuteEffect(const FGameplayTag& EffectTag, const FElementApplication& Context, AActor* AffectedActor);
	virtual void ExecuteEffect_Implementation(const FGameplayTag& EffectTag, const FElementApplication& Context, AActor* AffectedActor);
};
