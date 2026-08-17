#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "StealthPlayerCharacter.generated.h"

class UStealthCharacterCollisionsComponent;
class UInventoryComponent;
class UStealthCharacterAbilitiesComponent;
class UStealthCharacterAttributeSet;
class UStealthCharacterData;
class UAIPerceptionStimuliSourceComponent;
class UStealthCharacterInteractionComponent;
struct FInputActionValue;
class UInputAction;
class USkeletalMeshComponent;
class UCameraComponent;

UCLASS()
class STEALTH_API AStealthPlayerCharacter : public ACharacter, public IAbilitySystemInterface
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
	TObjectPtr<UStealthCharacterInteractionComponent> InteractionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStealthCharacterCollisionsComponent> CollisionsComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Stimuli", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSourceComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Ability System", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components|Ability System", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStealthCharacterAbilitiesComponent> StealthCharacterAbilitiesComponent;
#pragma endregion

#pragma region Input
	// TODO: Rework input binding to assign actions in a single place
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> JumpAction;
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditAnywhere, Category ="Input")
	TObjectPtr<UInputAction> CrouchAction;
	UPROPERTY(EditDefaultsOnly, Category ="Input")
	TObjectPtr<UInputAction> PrimaryInteractAction;
	UPROPERTY(EditDefaultsOnly, Category ="Input")
	TObjectPtr<UInputAction> SecondaryInteractAction;
	UPROPERTY(EditDefaultsOnly, Category ="Input")
	TObjectPtr<UInputAction> SprintAction;
#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UStealthCharacterAttributeSet> AttributeSet;

	/*Methods*/
public:
	AStealthPlayerCharacter();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	UFUNCTION(BlueprintCallable)
	AActor* TryDropItem(const TSubclassOf<AActor> ItemToDropClass) const;

public:
	[[nodiscard]] const TObjectPtr<UStealthCharacterAttributeSet>& GetAttributeSet() const { return AttributeSet; }

protected:
	void MoveInput(const FInputActionValue& Value);
	void LookInput(const FInputActionValue& Value);
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoPrimaryInteract();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSecondaryInteract();
};
