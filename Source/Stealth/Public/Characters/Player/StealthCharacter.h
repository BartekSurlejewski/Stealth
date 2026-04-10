#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StealthCharacter.generated.h"

class UStealthCharacterData;
class UAIPerceptionStimuliSourceComponent;
class UPlayerInteractionComponent;
struct FInputActionValue;
class UInputAction;
class USkeletalMeshComponent;
class UCameraComponent;

UCLASS()
class STEALTH_API AStealthCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerInteractionComponent> InteractionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Stimuli", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSourceComponent;
	UPROPERTY(VisibleAnywhere, Category="Components|Data")
	TObjectPtr<UStealthCharacterData> Data;

protected:
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> CrouchAction;
	UPROPERTY(EditDefaultsOnly, Category ="Input")
	TObjectPtr<UInputAction> InteractAction;

public:
	AStealthCharacter();

protected:
	virtual void BeginPlay() override;

	void MoveInput(const FInputActionValue& Value);

	void LookInput(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Input")
	void DoCrouchStart();
	virtual void DoCrouchStart_Implementation();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Input")
	void DoCrouchEnd();
	virtual void DoCrouchEnd_Implementation();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoInteract();

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
