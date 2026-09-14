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
	CMOVE_Vault UMETA(DisplayName = "Vault"),
	CMOVE_MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FSlideMoveParams
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float MinSpeed = 350;
	UPROPERTY(EditDefaultsOnly)
	float EnterImpulse = 500;
	UPROPERTY(EditDefaultsOnly)
	float GravityForce = 5000;
	UPROPERTY(EditDefaultsOnly)
	float Friction = 1.3f;
};

USTRUCT(BlueprintType)
struct FVaultMoveParams
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float MinSpeed = 350;
	UPROPERTY(EditDefaultsOnly)
	float EnterImpulse = 500;
	UPROPERTY(EditDefaultsOnly)
	float GravityForce = 5000;
	UPROPERTY(EditDefaultsOnly)
	float Friction = 1.3f;
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
	virtual void InitializeComponent() override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	UFUNCTION(BlueprintCallable)
	bool IsInCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;

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
