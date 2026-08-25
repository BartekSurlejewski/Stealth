#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ChemistryTypes.h"
#include "ChemistrySubsystem.generated.h"

class UChemistryReactionDataAsset;
class UChemistryEffect;
class AActor;

/**
 * World subsystem driving the Chemistry System.
 * Listens for FElementApplication messages via GameplayMessageSubsystem,
 * resolves Element + Material rules into Effects, and dispatches effects to targets/emitters.
 */
UCLASS()
class STEALTHCHEMISTRYSYSTEM_API UChemistrySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void OnWorldEndPlay(UWorld& InWorld) override;

	/**
	 * Broadcasts an element application message over the GameplayMessageSubsystem.
	 * This is the primary decoupled entry point for applying elements in the world.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chemistry")
	void ApplyElement(const FElementApplication& Application);

	/**
	 * Directly processes an element application without relying on message broadcasting.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chemistry")
	void ProcessElementApplication(const FElementApplication& Application);

	/**
	 * Helper function to retrieve all combined material tags from an Actor and its components.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chemistry")
	FGameplayTagContainer GetMaterialTagsForActor(const AActor* Actor, const UPrimitiveComponent* HitComponent = nullptr) const;

	/**
	 * Sets or changes the active reaction data asset at runtime, refreshing rules and effect handlers.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chemistry")
	void SetReactionDataAsset(UChemistryReactionDataAsset* InDataAsset);

	/**
	 * Dynamically registers a new reaction rule at runtime.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chemistry")
	void RegisterReactionRule(const FChemistryReactionRule& Rule);

	/**
	 * Dynamically registers an effect handler class for a given effect tag.
	 */
	UFUNCTION(BlueprintCallable, Category = "Chemistry")
	void RegisterEffectHandler(const FGameplayTag& EffectTag, TSubclassOf<UChemistryEffect> EffectClass);

	/**
	 * Returns the currently active reaction rules.
	 */
	UFUNCTION(BlueprintPure, Category = "Chemistry")
	const TArray<FChemistryReactionRule>& GetActiveReactionRules() const { return ActiveReactionRules; }

private:
	void LoadConfiguration();
	void RegisterMessageListener();
	void UnregisterMessageListener();

	void HandleElementAppliedMessage(FGameplayTag Channel, const FElementApplication& Payload);

	void EvaluateTargetReactions(AActor* TargetActor, const FElementApplication& Application);
	void EvaluateEmitterReactions(AActor* EmitterActor, const FElementApplication& Application);
	void EvaluateRadialReactions(const FElementApplication& Application);
	void EvaluateLocationReactions(const FElementApplication& Application);

	bool MatchesRule(const FChemistryReactionRule& Rule, const FGameplayTag& ElementTag, const FGameplayTagContainer& MaterialTags) const;
	void ApplyRuleEffects(const FChemistryReactionRule& Rule, const FElementApplication& Application, AActor* AffectedActor);
	void ExecuteSingleEffect(const FGameplayTag& EffectTag, const FElementApplication& Context, AActor* AffectedActor);

	UPROPERTY()
	TObjectPtr<UChemistryReactionDataAsset> ActiveDataAsset;

	UPROPERTY()
	TArray<FChemistryReactionRule> ActiveReactionRules;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UChemistryEffect>> InstantiatedEffectHandlers;

	FGameplayMessageListenerHandle MessageListenerHandle;
	FGameplayTag ActiveMessageChannelTag;
};
