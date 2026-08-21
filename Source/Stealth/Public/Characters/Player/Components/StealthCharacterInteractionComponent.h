#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Components/ActorComponent.h"
#include "StealthCharacterInteractionComponent.generated.h"


class IPickable;
class UStealthCharacterAbilitiesComponent;
class UInventoryComponent;
class UCameraComponent;
class UInputAction;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthCharacterInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

	/*Methods*/
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

	UFUNCTION(BlueprintCallable, Category = "Player|Interaction|Pickup")
	bool TryPickupItem(AActor* NewPickedItem) const;
	UFUNCTION(BlueprintCallable, Category = "Player|Interaction|Pickup")
	void DropPickedItem() const;
	UFUNCTION(BlueprintCallable, Category = "Player|Interaction|Pickup")
	void ThrowPickedItem() const;
	UFUNCTION()
	AActor* GetPickedItem() const { return PickedItem; }

	/*Properties*/
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Player|Interaction|Pickup")
	mutable TObjectPtr<AActor> PickedItem;

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
};
