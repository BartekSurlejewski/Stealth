#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ChemistryTypes.h"
#include "ChemistryReactionDataAsset.generated.h"

class UDataTable;
class UChemistryEffect;

/**
 * Primary DataAsset serving as the single source of truth for chemistry reaction rules
 * and effect handler registrations.
 */
UCLASS(BlueprintType)
class STEALTHCHEMISTRYSYSTEM_API UChemistryReactionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Reaction rules defining how Element + Material -> Effect(s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chemistry|Reactions")
	TArray<FChemistryReactionRule> ReactionRules;

	/** Optional external DataTable containing additional FChemistryReactionRule rows */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chemistry|Reactions")
	TObjectPtr<UDataTable> ReactionDataTable;

	/** Mapping of Effect Tag to UChemistryEffect handler class */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chemistry|Effects")
	TMap<FGameplayTag, TSubclassOf<UChemistryEffect>> EffectHandlers;

	/** Collects all reaction rules from both ReactionRules array and the optional ReactionDataTable */
	UFUNCTION(BlueprintCallable, Category = "Chemistry")
	void GetAllRules(TArray<FChemistryReactionRule>& OutRules) const;
};
