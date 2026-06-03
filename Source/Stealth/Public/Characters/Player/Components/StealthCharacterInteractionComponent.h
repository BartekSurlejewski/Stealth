#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthCharacterInteractionComponent.generated.h"


class UStealthCharacterAbilitiesComponent;
class UInventoryComponent;
class UCameraComponent;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableLookedAt, AActor*, LookedAtActor);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthCharacterInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnInteractableLookedAt OnInteractableLookedAt;

private:
	UPROPERTY(EditAnywhere, Category ="Pickup", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxInteractionDistance = 300.0f;
	UPROPERTY()
	TObjectPtr<UCameraComponent> CachedCamera;
	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComponent;
	UPROPERTY()
	TObjectPtr<UStealthCharacterAbilitiesComponent> AbilitiesComponent;
	UPROPERTY()
	TObjectPtr<AActor> LookAtInteractableActor;
	UPROPERTY()
	TObjectPtr<UInputAction> PrimaryInteractInputAction;
	UPROPERTY()
	TObjectPtr<UInputAction> SecondaryInteractInputAction;

public:
	UStealthCharacterInteractionComponent();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	FKey GetPrimaryInteractKey() const;

	UFUNCTION(BlueprintCallable)
	FKey GetSecondaryInteractKey() const;

protected:
	UFUNCTION()
	FKey GetKeyForInputAction(const UInputAction* InputAction) const;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION()
	AActor* GetLookAtInteractableActor() const;
	UFUNCTION()
	void Interact(bool bIsPrimaryInteraction) const;

	UFUNCTION()
	void InitializeInput(UInputAction* NewPrimaryInteractInputAction, UInputAction* NewSecondaryInteractInputAction)
	{
		PrimaryInteractInputAction = NewPrimaryInteractInputAction;
		SecondaryInteractInputAction = NewSecondaryInteractInputAction;
	}
};
