#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "Engine/EngineTypes.h"
#include "ChemistrySettings.generated.h"

class UChemistryReactionDataAsset;
class UDataTable;
class UChemistryEffect;

/**
 * Project settings for the Stealth Chemistry System.
 * Configured in Project Settings -> Game -> Stealth Chemistry System.
 * Allows editor-driven configuration with zero hardcoded gameplay tags.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Stealth Chemistry System"))
class STEALTHCHEMISTRYSYSTEM_API UChemistrySettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UChemistrySettings();

	/** Default message channel gameplay tag used with GameplayMessageSubsystem (e.g. Message.World.ElementApplied) */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Messages")
	FGameplayTag MessageChannelTag;

	/** Default reaction data asset containing reaction rules and effect handlers */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UChemistryReactionDataAsset> DefaultReactionDataAsset;

	/** Collision object channels queried during area/radial element applications */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Spatial")
	TArray<TEnumAsByte<ECollisionChannel>> RadialQueryCollisionChannels;

	virtual FName GetCategoryName() const override { return TEXT("Game"); }
};
