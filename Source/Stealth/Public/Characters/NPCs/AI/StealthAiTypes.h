#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "GameFramework/Pawn.h"
#include "StealthAiTypes.generated.h"

class AActor;

// ============================================================================
// AI Gameplay Tags Declarations
// ============================================================================

namespace StealthAiTags
{
	// NPC State Tags
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Unaware);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Suspicious);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Alerted);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Combat);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Search);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Dead);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_State_Fleeing);

	// NPC Activity Tags
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Activity_Patrol);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Activity_GuardPost);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Activity_Pub);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Activity_Sleep);

	// NPC Focus Tags
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_None);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Routine);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Noise_Distraction);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Disturbance_Environment);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Disturbance_DeadBody);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Player);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Player_Suspicious);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Focus_Player_Hostile);

	// NPC StateTree Event Tags
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_Investigate);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_NoiseHeard);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_CrimeReported);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_PlayerSpotted);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_PlayerLost);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_Distraction);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_FocusCleared);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_SearchExpired);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_ResumeRoutine);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_DamageReceived);
	STEALTH_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_NPC_Event_TouchDetected);
}

// ============================================================================
// Patrol & Noise Enums
// ============================================================================

UENUM(BlueprintType)
enum class EPatrolMode : uint8
{
	Loop,
	PingPong,
	Once
};

UENUM(BlueprintType)
enum class ENpcNoiseType : uint8
{
	Subtle = 0 UMETA(DisplayName = "Subtle"),
	Distraction = 1 UMETA(DisplayName = "Distraction"),
	Major = 2 UMETA(DisplayName = "Major"),
	Critical = 3 UMETA(DisplayName = "Critical")
};

// ============================================================================
// Sense Flags
// ============================================================================

UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ENpcSenseFlags : uint8
{
	None = 0 UMETA(Hidden),
	Sight = 1 << 0 UMETA(DisplayName = "Sight"),
	Hearing = 1 << 1 UMETA(DisplayName = "Hearing"),
	Touch = 1 << 2 UMETA(DisplayName = "Touch"),
	Damage = 1 << 3 UMETA(DisplayName = "Damage"),
	Prediction = 1 << 4 UMETA(DisplayName = "Prediction")
};
ENUM_CLASS_FLAGS(ENpcSenseFlags);

// ============================================================================
// Namespace NpcAi Utilities
// ============================================================================

namespace NpcAi
{
	inline bool IsPlayerActor(const AActor* Actor)
	{
		if (const APawn* Pawn = Cast<APawn>(Actor))
		{
			return Pawn->IsPlayerControlled();
		}
		return false;
	}
}

// ============================================================================
// Sensor Types
// ============================================================================

USTRUCT(BlueprintType)
struct STEALTH_API FNpcTrackedTarget
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	TWeakObjectPtr<AActor> Actor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	FVector LastKnownLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	bool bHasLocation = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	ENpcSenseFlags CurrentSenses = ENpcSenseFlags::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	double LastSeenTime = -1.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	double LastHeardTime = -1.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	double LastSensedTime = -1.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	FGameplayTag LastStimulusTag = FGameplayTag::EmptyTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	float LastStimulusStrength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	float Distance = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	bool bIsPlayer = false;

	bool IsCurrentlySeen() const
	{
		return EnumHasAnyFlags(CurrentSenses, ENpcSenseFlags::Sight);
	}

	double GetAge(double Now) const
	{
		return (LastSensedTime >= 0.0) ? (Now - LastSensedTime) : -1.0;
	}
};

USTRUCT(BlueprintType)
struct STEALTH_API FNpcSensorData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	TArray<FNpcTrackedTarget> Targets;

	static constexpr int32 MaxTrackedTargets = 16;

	const FNpcTrackedTarget* Find(const AActor* InActor) const
	{
		if (!InActor)
		{
			return nullptr;
		}
		return Targets.FindByPredicate([InActor](const FNpcTrackedTarget& Item) {
			return Item.Actor.Get() == InActor;
		});
	}

	FNpcTrackedTarget* Find(const AActor* InActor)
	{
		if (!InActor)
		{
			return nullptr;
		}
		return Targets.FindByPredicate([InActor](const FNpcTrackedTarget& Item) {
			return Item.Actor.Get() == InActor;
		});
	}

	FNpcTrackedTarget& FindOrAdd(AActor* InActor)
	{
		if (FNpcTrackedTarget* Found = Find(InActor))
		{
			return *Found;
		}

		if (Targets.Num() >= MaxTrackedTargets)
		{
			int32 OldestIndex = 0;
			double OldestTime = Targets[0].LastSensedTime;
			for (int32 i = 1; i < Targets.Num(); ++i)
			{
				if (Targets[i].LastSensedTime < OldestTime)
				{
					OldestTime = Targets[i].LastSensedTime;
					OldestIndex = i;
				}
			}
			Targets.RemoveAt(OldestIndex);
		}

		FNpcTrackedTarget NewTarget;
		NewTarget.Actor = InActor;
		NewTarget.bIsPlayer = NpcAi::IsPlayerActor(InActor);
		return Targets.Add_GetRef(NewTarget);
	}

	bool Remove(const AActor* InActor)
	{
		if (!InActor)
		{
			return false;
		}
		const int32 NumRemoved = Targets.RemoveAll([InActor](const FNpcTrackedTarget& Item) {
			return Item.Actor.Get() == InActor || !Item.Actor.IsValid();
		});
		return NumRemoved > 0;
	}

	void PruneInvalid()
	{
		Targets.RemoveAll([](const FNpcTrackedTarget& Item) {
			return !Item.Actor.IsValid();
		});
	}
};

UENUM(BlueprintType)
enum class ENpcSensorEventType : uint8
{
	SightGained,
	SightLost,
	Heard,
	Touched,
	Damaged,
	Forgotten
};

USTRUCT(BlueprintType)
struct STEALTH_API FNpcSensorEvent
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	ENpcSensorEventType Type = ENpcSensorEventType::SightGained;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	TWeakObjectPtr<AActor> Actor = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	float Strength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	FGameplayTag Tag = FGameplayTag::EmptyTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	ENpcNoiseType NoiseType = ENpcNoiseType::Subtle;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNpcSensorEvent, const FNpcSensorEvent&);

// ============================================================================
// Awareness Types
// ============================================================================

UENUM(BlueprintType)
enum class ENpcSuspicionMode : uint8
{
	Normal UMETA(DisplayName = "Normal"),
	FrozenWhileVisible UMETA(DisplayName = "Frozen While Visible"),
	AlwaysDecay UMETA(DisplayName = "Always Decay")
};

USTRUCT(BlueprintType)
struct STEALTH_API FNpcPerceptionTuningParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Tuning")
	ENpcSuspicionMode Mode = ENpcSuspicionMode::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Tuning", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SuspicionGainMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Tuning", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SuspicionDecayMultiplier = 1.0f;
};

UENUM(BlueprintType)
enum class ENpcVisibilityGrade : uint8
{
	Hidden,
	Shadowed,
	PartiallyExposed,
	FullyVisible
};

USTRUCT(BlueprintType)
struct STEALTH_API FNpcAwarenessState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Awareness")
	float Suspicion = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Awareness")
	ENpcVisibilityGrade VisibilityGrade = ENpcVisibilityGrade::Hidden;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Awareness")
	bool bFocusTargetVisible = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Awareness")
	bool bFocusTargetStale = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Awareness")
	bool bHasInvestigationLocation = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Awareness")
	FVector InvestigationLocation = FVector::ZeroVector;
};

// ============================================================================
// Focus Types
// ============================================================================

UENUM(BlueprintType)
enum class ENpcFocusPriority : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Ambient = 10 UMETA(DisplayName = "Ambient"),
	Routine = 15 UMETA(DisplayName = "Routine"),
	MinorDistraction = 20 UMETA(DisplayName = "Minor Distraction"),
	MajorDisturbance = 30 UMETA(DisplayName = "Major Disturbance"),
	SuspiciousPlayer = 25 UMETA(DisplayName = "Suspicious Player"),
	CriticalDisturbance = 40 UMETA(DisplayName = "Critical Disturbance"),
	ThreatInSight = 50 UMETA(DisplayName = "Threat In Sight"),
	CombatTarget = 60 UMETA(DisplayName = "Combat Target")
};

USTRUCT(BlueprintType)
struct STEALTH_API FNpcFocusRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	FGameplayTag FocusTag = FGameplayTag::EmptyTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	ENpcFocusPriority Priority = ENpcFocusPriority::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	float AwarenessReductionMultiplier = 1.0f;
};

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
	bool bHasLocation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	float RemainingTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Focus")
	float AwarenessReductionMultiplier = 1.0f;

	bool IsValid() const
	{
		return Priority > ENpcFocusPriority::None && (FocusActor.IsValid() || bHasLocation);
	}

	bool operator==(const FNpcFocusTarget& Other) const
	{
		return FocusTag == Other.FocusTag
			&& Priority == Other.Priority
			&& FocusActor == Other.FocusActor
			&& bHasLocation == Other.bHasLocation
			&& (!bHasLocation || FocusLocation.Equals(Other.FocusLocation, 1.0f));
	}

	bool operator!=(const FNpcFocusTarget& Other) const
	{
		return !(*this == Other);
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNpcFocusChanged, const FNpcFocusTarget&, NewFocus, const FNpcFocusTarget&, PreviousFocus);
