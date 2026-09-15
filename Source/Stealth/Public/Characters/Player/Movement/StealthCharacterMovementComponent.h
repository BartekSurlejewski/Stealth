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

	UPROPERTY(EditDefaultsOnly)
	float MaxDistance = 200;
	UPROPERTY(EditDefaultsOnly)
	float ReachHeight = 50;
	UPROPERTY(EditDefaultsOnly)
	float MinDepth = 30;
	UPROPERTY(EditDefaultsOnly)
	float MinWallSteepnessAngle = 75;
	UPROPERTY(EditDefaultsOnly)
	float MaxSurfaceAngle = 40;
	UPROPERTY(EditDefaultsOnly)
	float MaxAlignmentAngle = 45;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* TallVaultMontage;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* TransitionTallVaultMontage;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ProxyTallVaultMontage;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ShortVaultMontage;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* TransitionShortVaultMontage;
	UPROPERTY(EditDefaultsOnly)
	UAnimMontage* ProxyShortVaultMontage;
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
	UPROPERTY(EditDefaultsOnly, Category="Custom Movement|Slide")
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
	UPROPERTY(EditDefaultsOnly, Category="Custom Movement|Vault")
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
