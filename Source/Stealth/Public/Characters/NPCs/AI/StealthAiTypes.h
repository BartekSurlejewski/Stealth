#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "Housing/HouseData.h"
#include "StealthAiTypes.generated.h"

class AActor;
class ANpcCharacter;
class ANpcAiController;

namespace StealthAiTags
{
	// NPC State Tags (Single Source of Truth)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Unaware);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Suspicious);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Alerted);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Combat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Search);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Fleeing);

	// NPC Event / Trigger Tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_Investigate);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_NoiseHeard);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_CrimeReported);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_PlayerSpotted);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_PlayerLost);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_Distraction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_FocusCleared);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_SearchExpired);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_ResumeRoutine);

	// Player Status Tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_State_Illegal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_State_Trespassing);

	// NPC Activity Tags (Schedule & Routines)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Activity_Patrol);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Activity_GuardPost);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Activity_Pub);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Activity_Sleep);

	// Noise Tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Noise_Footstep);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Noise_Distraction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Noise_Major);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Noise_Critical);

	// NPC Focus Tags
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_None);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Routine);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Noise_Distraction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Disturbance_Environment);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Disturbance_DeadBody);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Player_Suspicious);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Player_Hostile);
}

/**
 * Behavior and Alert level of an NPC
 */
UENUM(BlueprintType)
enum class ENpcAlertLevel : uint8
{
	Unaware UMETA(DisplayName = "Unaware / Routine"),
	Suspicious UMETA(DisplayName = "Suspicious / Inquiring"),
	Alerted UMETA(DisplayName = "Alerted / Investigating"),
	Hostile UMETA(DisplayName = "Hostile / Combat / Alarm")
};

UENUM(BlueprintType)
enum class ENpcBehaviourState : uint8
{
	Routine UMETA(DisplayName = "Routine / Patrol / Schedule"),
	Suspicious UMETA(DisplayName = "Suspicious"),
	Alerted UMETA(DisplayName = "Alerted"),
	Search UMETA(DisplayName = "Search"),
	Combat UMETA(DisplayName = "Combat / Alarm")
};

// Typedef alias for backward compatibility
using EGuardBehaviourState = ENpcBehaviourState;

/**
 * Noise classification for hearing stimuli
 */
UENUM(BlueprintType)
enum class ENpcNoiseType : uint8
{
	Subtle UMETA(DisplayName = "Subtle / Footsteps"),
	Distraction UMETA(DisplayName = "Distraction / Thrown Rock"),
	Major UMETA(DisplayName = "Major Disturbance / Glass Shatter"),
	Critical UMETA(DisplayName = "Critical / Explosion / Death Scream")
};

/**
 * Patrol traversal pattern for patrol routes
 */
UENUM(BlueprintType)
enum class EPatrolMode : uint8
{
	Loop UMETA(DisplayName = "Loop (0 -> 1 -> 2 -> 0)"),
	PingPong UMETA(DisplayName = "Ping-Pong (0 -> 1 -> 2 -> 1 -> 0)"),
	Once UMETA(DisplayName = "Once (Stop at last waypoint)"),
	Random UMETA(DisplayName = "Random Waypoint")
};

/**
 * Focus priority hierarchy: Higher numerical values preempt lower ones.
 * Ordering is behaviorally load-bearing: it gates whether an NPC notices or reacts to a player
 * while engaged in other activities.
 */
UENUM(BlueprintType)
enum class ENpcFocusPriority : uint8
{
	None = 0,
	Routine = 10,              // Daily activity / idle facing
	MinorDistraction = 20,     // Soft noise, rock throw, whistling
	SuspiciousPlayer = 25,     // Player spotted / suspicious presence (sub-disturbance, ignored if busy with major tasks)
	MajorDisturbance = 30,     // Open door in secure area, blood, missing item
	CriticalDisturbance = 40,  // Dead / incapacitated body
	CombatTarget = 60          // Confirmed hostile player in combat / illegal act seen (highest priority)
};

/**
 * Immutable per-tick context passed to UNpcState virtual evaluation methods.
 * Keeps state logic free of component-internal lookups and easy to unit test.
 */
USTRUCT(BlueprintType)
struct STEALTH_API FNpcStateTickContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float DeltaTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float CurrentSuspicion = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	bool bHasLineOfSight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	bool bEffectivelySeesPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	bool bIsPlayerPerformingIllegalAction = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	bool bIsFocusingOnPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float FocusAwarenessMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float PlayerDistance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float PlayerExposureMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	bool bIsLookingDirectlyAtPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float LostSightDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float SearchDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float BaseGainRateSight = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float BaseGainRatePeripheral = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float BaseDecayRate = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float AlertThreshold = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float SuspiciousThreshold = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float SightLossGrace = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|State")
	float AlwaysSeeRange = 100.0f;
};

/**
 * Snapshot of what the NPC is currently focusing on
 */
USTRUCT(BlueprintType)
struct STEALTH_API FNpcFocusTarget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	FGameplayTag FocusTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	ENpcFocusPriority Priority = ENpcFocusPriority::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	TWeakObjectPtr<AActor> FocusActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	FVector FocusLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	float Duration = 0.0f; // 0 = indefinite until cleared or preempted

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	float RemainingTime = 0.0f;

	/** Modifier applied to peripheral vision / pickpocket difficulty while focused (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	float AwarenessReductionMultiplier = 0.5f;

	bool IsValid() const
	{
		return Priority > ENpcFocusPriority::None && (FocusActor.IsValid() || !FocusLocation.IsZero());
	}

	bool operator==(const FNpcFocusTarget& Other) const
	{
		return FocusTag == Other.FocusTag
			&& Priority == Other.Priority
			&& FocusActor == Other.FocusActor
			&& FocusLocation.Equals(Other.FocusLocation, 1.0f);
	}

	bool operator!=(const FNpcFocusTarget& Other) const
	{
		return !(*this == Other);
	}
};

/**
 * Perception snapshot passed to StateTree and listeners
 */
USTRUCT(BlueprintType)
struct FStateTreeAiPerceptionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float Awareness = 0.0f; // 0.0 to 100.0

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	ENpcAlertLevel AlertLevel = ENpcAlertLevel::Unaware;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	bool bHasPlayerLineOfSight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	bool bEffectivelySeesPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	FVector LastKnownPlayerPosition = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	FVector LastHeardSoundLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	float TimeSinceLastStimulus = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	TWeakObjectPtr<AActor> LastPerceivedActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Perception")
	FGameplayTag LastStimulusTag;
};

/**
 * Structured payload for crime events dispatched across AI systems
 */
USTRUCT(BlueprintType)
struct FAiCrimeEventPayload
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crime")
	FName HouseId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crime")
	EHouseCrimeType CrimeType = EHouseCrimeType::Trespassing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crime")
	FVector CrimeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crime")
	TWeakObjectPtr<AActor> Perpetrator = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crime")
	TWeakObjectPtr<AActor> VictimOrTarget = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Crime")
	bool bIsPrimaryInvestigator = false;
};

/**
 * Information about an active activity slot / smart object
 */
USTRUCT(BlueprintType)
struct FActivitySlotClaim
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "AI|Activity")
	FGuid ClaimerGuid;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Activity")
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Activity")
	FGameplayTag ActivityTag;

	UPROPERTY(BlueprintReadOnly, Category = "AI|Activity")
	float ClaimTimestamp = 0.0f;
};
