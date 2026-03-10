// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/PlayerInteractionComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Characters/Player/StealthCharacter.h"
#include "GameFramework/Character.h"
#include "Interactables/Interactable.h"

// Sets default values for this component's properties
UPlayerInteractionComponent::UPlayerInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

FKey UPlayerInteractionComponent::GetCurrentInteractKey() const
{
	return GetKeyForInputAction(InteractInputAction);
}


void UPlayerInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		CachedCamera = OwnerCharacter->FindComponentByClass<UCameraComponent>();

		if (!CachedCamera)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Interaction Component] No camera found on %s"), *OwnerCharacter->GetName());
		}
	}

	OnInteractableLookedAt.Broadcast(LookAtInteractableActor);
}

FKey UPlayerInteractionComponent::GetKeyForInputAction(const UInputAction* InputAction) const
{
	if (!InputAction)
	{
		return FKey();
	}

	APlayerController* PlayerController = Cast<APlayerController>(GetOwner()->GetInstigatorController());
	if (!PlayerController)
	{
		return FKey();
	}

	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	if (!EnhancedInputSubsystem)
	{
		return FKey();
	}

	TArray<FKey> Keys = EnhancedInputSubsystem->QueryKeysMappedToAction(InputAction);

	if (Keys.Num() > 0)
	{
		return Keys[0];
	}

	return FKey();
}


// Called every frame
void UPlayerInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AActor* NewLookAtInteractableActor = GetLookAtInteractableActor();

	if (NewLookAtInteractableActor != LookAtInteractableActor)
	{
		if (LookAtInteractableActor)
		{
			IInteractable::Execute_SetHighlighted(Cast<UObject>(LookAtInteractableActor), false);
		}

		if (NewLookAtInteractableActor)
		{
			IInteractable::Execute_SetHighlighted(Cast<UObject>(NewLookAtInteractableActor), true);
		}

		LookAtInteractableActor = NewLookAtInteractableActor;
		OnInteractableLookedAt.Broadcast(LookAtInteractableActor);
	}
}

AActor* UPlayerInteractionComponent::GetLookAtInteractableActor() const
{
	FHitResult HitResult;

	const FVector Start = CachedCamera->GetComponentLocation();
	const FVector End = Start + CachedCamera->GetForwardVector() * MaxInteractionDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor && HitActor->Implements<UInteractable>())
		{
			return HitActor;
		}
	}

	return nullptr;
}

void UPlayerInteractionComponent::Interact() const
{
	if (!LookAtInteractableActor || !LookAtInteractableActor->Implements<UInteractable>())
	{
		return;
	}

	// IInteractable* LookAtInteractable = Cast<IInteractable>(LookAtInteractableActor);
	// if (!LookAtInteractable)
	// {
	// 	return;
	// }

	IInteractable::Execute_Interact(LookAtInteractableActor, Cast<AStealthCharacter>(GetOwner()));
}
