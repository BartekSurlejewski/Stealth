// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInteractionComponent.generated.h"


class UCameraComponent;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableLookedAt, AActor*, LookedAtActor);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UPlayerInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FOnInteractableLookedAt OnInteractableLookedAt;

private:
	UPROPERTY(EditAnywhere, Category ="Pickup", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxInteractionDistance = 300.0f;
	UPROPERTY()
	TSoftObjectPtr<UCameraComponent> CachedCamera;
	UPROPERTY()
	TObjectPtr<AActor> LookAtInteractableActor;
	UPROPERTY()
	TObjectPtr<UInputAction> InteractInputAction;
	UPROPERTY()
	FKey InteractInputKey;

public:
	UPlayerInteractionComponent();

protected:
	virtual void BeginPlay() override;
	UFUNCTION()
	FKey GetKeyForInputAction(const UInputAction* InputAction) const;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UFUNCTION()
	AActor* GetLookAtInteractableActor() const;
	UFUNCTION()
	void Interact();

	UFUNCTION()
	void SetInteractInputAction(UInputAction* NewInteractInputAction)
	{
		if (NewInteractInputAction != InteractInputAction)
		{
			InteractInputKey = GetKeyForInputAction(NewInteractInputAction);
		}

		InteractInputAction = NewInteractInputAction;
	}
};
