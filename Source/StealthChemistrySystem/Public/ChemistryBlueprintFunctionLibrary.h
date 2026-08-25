#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "ChemistryTypes.h"
#include "ChemistryBlueprintFunctionLibrary.generated.h"

class AActor;

/**
 * Blueprint function library offering convenient static helpers for the Chemistry System.
 */
UCLASS()
class STEALTHCHEMISTRYSYSTEM_API UChemistryBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Broadcasts an element application in the world via the Chemistry System.
	 * Decoupled way for any actor (projectile, spell, tool, weapon, trap) to emit an element.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chemistry", meta = (WorldContext = "WorldContextObject"))
	static void ApplyElement(const UObject* WorldContextObject, const FElementApplication& Application);

	/**
	 * Convenience helper to create and broadcast an element application at a target actor.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chemistry", meta = (WorldContext = "WorldContextObject"))
	static void ApplyElementToActor(const UObject* WorldContextObject, FGameplayTag ElementTag, AActor* TargetActor, AActor* Instigator = nullptr, float Magnitude = 1.0f);

	/**
	 * Convenience helper to create and broadcast a radial element application at a world location.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chemistry", meta = (WorldContext = "WorldContextObject"))
	static void ApplyElementInRadius(const UObject* WorldContextObject, FGameplayTag ElementTag, FVector Location, float Radius, AActor* Instigator = nullptr,
	                                 float Magnitude = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Chemistry", meta = (WorldContext = "WorldContextObject"))
	static void ApplyElementFromHitResult(const UObject* WorldContextObject, FGameplayTag ElementTag, const FHitResult& Hit, AActor* Instigator, float Magnitude = 1.0f);

	/**
	 * Returns all combined material tags from an actor and its components.
	 */
	UFUNCTION(BlueprintPure, Category = "Chemistry")
	static FGameplayTagContainer GetActorMaterialTags(const AActor* Actor);
};
