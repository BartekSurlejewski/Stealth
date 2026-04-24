#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "StealthCharacter.generated.h"

class UStealthCharacterAttributeSet;
class UStealthCharacterData;
class UAIPerceptionStimuliSourceComponent;
class UPlayerInteractionComponent;
struct FInputActionValue;
class UInputAction;
class USkeletalMeshComponent;
class UCameraComponent;

UCLASS()
class STEALTH_API AStealthCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
	/*Properties*/
protected:
#pragma region Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPlayerInteractionComponent> InteractionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Stimuli", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSourceComponent;
	UPROPERTY(VisibleAnywhere, Category="Components|Data", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStealthCharacterData> Data;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Ability System", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
#pragma endregion

#pragma region Input
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
	UPROPERTY(EditDefaultsOnly, Category ="Input")
	TObjectPtr<UInputAction> SprintAction;
#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UStealthCharacterAttributeSet> AttributeSet;

	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag MovingTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag StaminaRegenTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag SprintAbilityTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags")
	FGameplayTag CrouchAbilityTag;

	UPROPERTY()
	FGameplayTagContainer SprintAbilityTagsContainer;
	UPROPERTY()
	FGameplayTagContainer CrouchAbilityTagsContainer;
	/*Methods*/
public:
	AStealthCharacter();

	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
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

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoInteract();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintInputStart();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintInputEnd();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchInputStart();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchInputEnd();

private:
	UFUNCTION()
	void UpdateTags() const;
	UFUNCTION()
	void EndSprint() const;
};
