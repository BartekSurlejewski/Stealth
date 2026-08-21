#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "ChemistryTypes.generated.h"

class AActor;

/**
 * Defines which actor(s) the resulting reaction effects should be applied to.
 */
UENUM(BlueprintType)
enum class EChemistryReactionTarget : uint8
{
	/** Apply effect to the target actor (or location if no actor) */
	TargetOnly UMETA(DisplayName = "Target Only"),

	/** Apply effect to the emitter/instigator actor */
	EmitterOnly UMETA(DisplayName = "Emitter Only"),

	/** Apply effect to both target and emitter actors */
	BothTargetAndEmitter UMETA(DisplayName = "Both Target And Emitter"),

	/** Apply effect at the application location without specific actor targeting */
	LocationOnly UMETA(DisplayName = "Location Only")
};

/**
 * Message payload describing an element applied to a target or area.
 * Broadcast via GameplayMessageSubsystem or applied directly through UChemistrySubsystem.
 */
USTRUCT(BlueprintType)
struct STEALTHCHEMISTRYSYSTEM_API FElementApplication
{
	GENERATED_BODY()

	/** The element medium or force applied (e.g., Element.Fire, Element.Force, Element.Water) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	FGameplayTag ElementTag;

	/** The actor that emitted or initiated this element application */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	TWeakObjectPtr<AActor> Instigator = nullptr;

	/** Optional specific target actor hit by the element */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	/** World location where the element was applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	FVector Location = FVector::ZeroVector;

	/** Impact or intensity magnitude (e.g., speed, damage factor, heat level) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	float Magnitude = 1.0f;

	/** Radius of effect for area-of-effect element application (0 for point/single-target) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	float Radius = 0.0f;

	/** Additional optional gameplay tags describing context (e.g. wetness level, direction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	FGameplayTagContainer ContextTags;

	/** Optional arbitrary context object passed with the application */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	TObjectPtr<UObject> OptionalContextObject = nullptr;
};

/**
 * Data-driven rule defining how an applied Element interacts with a Target/Emitter Material to produce Effects.
 */
USTRUCT(BlueprintType)
struct STEALTHCHEMISTRYSYSTEM_API FChemistryReactionRule : public FTableRowBase
{
	GENERATED_BODY()

	/** The element tag that triggers this reaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	FGameplayTag ElementTag;

	/** Required material tags on the target for this reaction to trigger. If empty, matches any material or targetless application */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	FGameplayTagContainer RequiredMaterialTags;

	/** If true, the target must have ALL required material tags; if false, ANY matching tag suffices */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	bool bRequireAllMaterialTags = false;

	/** Material tags that prevent this reaction from triggering if present on the target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	FGameplayTagContainer BlockedMaterialTags;

	/** The effect tags resulting from this reaction (e.g., Effect.Destroy, Effect.Ignite, Effect.Knockback) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	FGameplayTagContainer ResultingEffects;

	/** Defines who receives the resulting effects (Target, Emitter, Both, or Location) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	EChemistryReactionTarget ReactionTarget = EChemistryReactionTarget::TargetOnly;

	/** Multiplier applied to context magnitude when executing resulting effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chemistry")
	float EffectMagnitudeMultiplier = 1.0f;
};
