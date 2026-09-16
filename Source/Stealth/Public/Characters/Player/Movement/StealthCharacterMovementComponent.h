#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StealthCharacterMovementComponent.generated.h"

class AStealthPlayerCharacter;

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None UMETA(Hidden),
	CMOVE_Slide UMETA(DisplayName = "Slide"),
	CMOVE_MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FSlideMoveParams
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float MinSpeed = 150;
	UPROPERTY(EditDefaultsOnly)
	float EnterImpulse = 500;
	UPROPERTY(EditDefaultsOnly)
	float GravityForce = 5000;
	UPROPERTY(EditDefaultsOnly)
	float Friction = 1.3f;
	UPROPERTY(EditDefaultsOnly)
	float BrakingDeceleration = 1000.0f;
	UPROPERTY(EditDefaultsOnly)
	float CapsuleHalfHeight = 32.0f;
};

USTRUCT(BlueprintType)
struct FVaultMoveParams
{
	GENERATED_BODY()

	/**Determines how many traces will be fired to find vaultable obstacle.
	 * Keep this value as low as possible for performance reasons.
	 */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "1"))
	int VaultTracesCount = 9;

	/** Max forward trace distance (cm) used to detect a vaultable obstacle ahead.
	 *  Scales with forward speed (clamped between CapsuleRadius+30 and this value).
	 *  Also used as the divisor when computing transition duration, so it indirectly
	 *  controls how "snappy" long vaults feel. */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", Units = "cm"))
	float MaxDistance = 200;

	/** Extra height (cm) above the character's full standing height that counts as
  *  "reachable". Effectively sets the tallest obstacle the character can vault:
  *  MaxVaultHeight = CapsuleFullHeight + ReachHeight. */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", Units = "cm"))
	float ReachHeight = 50;

	/**  The threshold of how much taller (or lower) than the character's capsule a wall needs to be to be treated as tall */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0"))
	float TallVaultHeightThresholdMultiplier = 1.2f;

	/** Minimum steepness (degrees from horizontal) a hit surface must have to count
	*  as a vaultable "wall" rather than a ramp/floor. Lower = also accepts shallower
	*  sloped surfaces as vault walls; higher = requires a near-vertical wall. */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "15", ClampMax = "90", Units = "deg"))
	float MinWallSteepnessAngle = 75;

	/** Maximum steepness (degrees from horizontal) allowed for the TOP surface the
	*  character will land on. Surfaces tilted more than this are rejected as unsafe
	*  landing spots (e.g. a sloped rooftop). */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "90", Units = "deg"))
	float MaxSurfaceAngle = 40;

	/** Maximum angle (degrees) between the character's facing direction and the
   *  wall's surface normal for a vault to trigger. Lower values require a more
   *  direct, head-on approach. */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", ClampMax = "90", Units = "deg"))
	float MaxAlignmentAngle = 45;

	/** For low/short obstacles only: how far (cm) past the obstacle's front face to
  *  probe for a walkable floor. If found and clear, the character jumps straight
  *  through/over the obstacle instead of mounting on top of it. */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", Units = "cm"))
	float JumpThroughDistance = 100;

	/** Minimum duration (seconds) of the root-motion vault transition, even for very
   *  short vaults. Prevents the move from feeling instant/teleport-y. */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", Units = "s"))
	float MinTransitionTime = 0.1f;

	/** Maximum duration (seconds) of the vault transition, even for very long vaults.
	*  Prevents the move from feeling sluggish over long distances. */
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0", Units = "s"))
	float MaxTransitionTime = 0.25f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomMovementModeEntered, ECustomMovementMode, EnteredMovementMode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomMovementModeExit, ECustomMovementMode, ExitMovementMode);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	/*Events*/
public:
	UPROPERTY(BlueprintAssignable)
	FCustomMovementModeEntered OnCustomMovementModeEntered;
	UPROPERTY(BlueprintAssignable)
	FCustomMovementModeExit OnCustomMovementModeExit;

public:
	UStealthCharacterMovementComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma region  Movement Flow
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
#pragma endregion

#pragma region  Movement Helpers
	virtual bool IsMovingOnGround() const override;
	virtual bool CanAttemptJump() const override;
	virtual float GetMaxBrakingDeceleration() const override;

	// CAPSULE RESIZING
	UFUNCTION()
	bool ResizeCapsuleSize(float NewHalfHeight);
	UFUNCTION()
	bool RestoreDefaultCapsuleSize();

	UFUNCTION(BlueprintCallable)
	bool IsInCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	UFUNCTION(BlueprintCallable)
	bool IsInMovementMode(EMovementMode InMovementMode) const;
#pragma endregion

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<AStealthPlayerCharacter> PlayerCharacterOwner;

#pragma region Slide

protected:
	UPROPERTY(EditDefaultsOnly, Category="Stealth Movement|Slide")
	FSlideMoveParams SlideMoveParams;

private:
	UPROPERTY()
	bool bWantsToSlide = false;

	//Transient
	TSharedPtr<FRootMotionSource_MoveToForce> TransitionRMS;
	UPROPERTY(Transient)
	FString TransitionName;
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> TransitionQueuedMontage;
	UPROPERTY(Transient)
	float TransitionQueuedMontageSpeed;
	UPROPERTY(Transient)
	int TransitionRMS_ID;

public:
	UFUNCTION(BlueprintCallable)
	void SetSlide(bool bNewWantsToSlide);

private:
	UFUNCTION()
	void EnterSlide();
	UFUNCTION()
	void ExitSlide();
	UFUNCTION()
	bool CanSlide() const;
	UFUNCTION()
	void PhysSlide(float DeltaTime, int32 Iterations);
#pragma endregion

#pragma region Vault

protected:
	UPROPERTY(EditDefaultsOnly, Category="Stealth Movement|Vault")
	FVaultMoveParams VaultMoveParams;

private:
	UPROPERTY()
	bool bIsJumpInputActive = false;

public:
	UFUNCTION(BlueprintCallable)
	void SetJumpInputActive(bool bNewIsActive);

private:
	UFUNCTION()
	bool TryVault();
	UFUNCTION()
	FVector GetVaultStartLocation(FHitResult FrontHit, FHitResult SurfaceHit, bool bTallVault) const;
#pragma endregion
};
