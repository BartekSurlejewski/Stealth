#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ChemistryTypes.h"
#include "ChemistryReceiverInterface.h"
#include "ChemistryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChemistryEffectReceivedSignature, const FGameplayTag&, EffectTag, const FElementApplication&, Context);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnElementAppliedSignature, const FElementApplication&, Context);

/**
 * Component that gives an Actor material properties and chemistry reaction capabilities.
 */
UCLASS(ClassGroup = (Chemistry), meta = (BlueprintSpawnableComponent))
class STEALTHCHEMISTRYSYSTEM_API UChemistryComponent : public UActorComponent, public IChemistryReceiverInterface
{
	GENERATED_BODY()

public:
	UChemistryComponent();

	/** Material tags assigned to this actor (e.g., Material.Wood, Material.Flammable, Material.Fragile) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	FGameplayTagContainer MaterialTags;

	/**
	 * Optional. If set, this component's material tags only apply when this exact primitive
	 * was struck (e.g. the bulb static mesh). Leave unset for actor-wide tags — unchanged,
	 * backward-compatible default behavior.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	TWeakObjectPtr<UPrimitiveComponent> AssociatedPrimitiveComponent = nullptr;

	/** Delegate fired when a chemistry effect is received by this actor */
	UPROPERTY(BlueprintAssignable, Category = "Chemistry")
	FOnChemistryEffectReceivedSignature OnEffectReceived;

	/** Delegate fired when an element is directly applied to this actor */
	UPROPERTY(BlueprintAssignable, Category = "Chemistry")
	FOnElementAppliedSignature OnElementApplied;

	// IChemistryReceiverInterface
	virtual FGameplayTagContainer GetMaterialTags_Implementation() const override;
	virtual bool OnReceiveChemistryEffect_Implementation(const FGameplayTag& EffectTag, const FElementApplication& Context) override;
	virtual void OnReceiveElementApplication_Implementation(const FElementApplication& Context) override;

	/** Adds a material tag at runtime */
	UFUNCTION(BlueprintCallable, Category = "Chemistry")
	void AddMaterialTag(const FGameplayTag& Tag);

	/** Removes a material tag at runtime */
	UFUNCTION(BlueprintCallable, Category = "Chemistry")
	void RemoveMaterialTag(const FGameplayTag& Tag);

	/** Checks if this component currently has a specific material tag */
	UFUNCTION(BlueprintCallable, Category = "Chemistry")
	bool HasMaterialTag(const FGameplayTag& Tag) const;
};
