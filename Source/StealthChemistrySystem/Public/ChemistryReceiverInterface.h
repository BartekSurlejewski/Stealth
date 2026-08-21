#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "ChemistryTypes.h"
#include "ChemistryReceiverInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UChemistryReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface implemented by Actors or Components that participate in the Chemistry System.
 * Allows providing material tags and implementing custom reaction logic to incoming effects or element applications.
 */
class STEALTHCHEMISTRYSYSTEM_API IChemistryReceiverInterface
{
	GENERATED_BODY()

public:
	/**
	 * Returns the material tags intrinsic to this actor/component (e.g., Material.Wood, Material.Flammable, Material.Metal).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chemistry")
	FGameplayTagContainer GetMaterialTags() const;

	/**
	 * Called when an effect is applied to this actor (e.g. Effect.Ignite, Effect.Destroy, Effect.Knockback).
	 * Enables custom reactions such as playing custom animations, starting particle emitters, or altering actor state.
	 * @param EffectTag The tag of the effect being applied.
	 * @param Context The original element application context.
	 * @return True if the effect was handled / overridden by this actor, false to allow default effect handler to also run.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chemistry")
	bool OnReceiveChemistryEffect(const FGameplayTag& EffectTag, const FElementApplication& Context);

	/**
	 * Called when an element is applied to this actor prior to or during reaction evaluation.
	 * @param Context The element application context.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Chemistry")
	void OnReceiveElementApplication(const FElementApplication& Context);
};
