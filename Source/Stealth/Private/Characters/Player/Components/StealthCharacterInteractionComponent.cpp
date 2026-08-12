#include "Characters/Player/Components/StealthCharacterInteractionComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Abilities/GameplayAbility.h"
#include "Camera/CameraComponent.h"
#include "Characters/Player/StealthCharacter.h"
#include "Characters/Player/Components/StealthCharacterAbilitiesComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Environment/Interactables/Interactable.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/Pickable.h"
#include "Messages/StealthMessages.h"

UStealthCharacterInteractionComponent::UStealthCharacterInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

FKey UStealthCharacterInteractionComponent::GetPrimaryInteractKey() const
{
	return GetKeyForInputAction(PrimaryInteractInputAction);
}

FKey UStealthCharacterInteractionComponent::GetSecondaryInteractKey() const
{
	return GetKeyForInputAction(SecondaryInteractInputAction);
}

void UStealthCharacterInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		CachedCamera = OwnerCharacter->FindComponentByClass<UCameraComponent>();

		if (!CachedCamera)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Interaction Component] No camera found on %s"), *OwnerCharacter->GetName());
		}

		//TODO: Think of some better way of passing interacted item to inventory for separation of concerns
		InventoryComponent = OwnerCharacter->GetPlayerState()->FindComponentByClass<UInventoryComponent>();
		AbilitiesComponent = OwnerCharacter->FindComponentByClass<UStealthCharacterAbilitiesComponent>();
	}
}

FKey UStealthCharacterInteractionComponent::GetKeyForInputAction(const UInputAction* InputAction) const
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

	if (!Keys.IsEmpty())
	{
		return Keys[0];
	}

	return FKey();
}

void UStealthCharacterInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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

		UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
		const FInteractableMessage Message(LookAtInteractableActor);
		MsgSubsystem.BroadcastMessage(StealthMessageChannels::TAG_Message_Player_LookedAtInteractable.GetTag(), Message);
	}
}

AActor* UStealthCharacterInteractionComponent::GetLookAtInteractableActor() const
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

void UStealthCharacterInteractionComponent::Interact(bool bIsPrimaryInteraction) const
{
	if (!LookAtInteractableActor || !LookAtInteractableActor->Implements<UInteractable>())
	{
		return;
	}

	FGameplayTag RequiredAbilityTag = bIsPrimaryInteraction
		                                  ? IInteractable::Execute_GetPrimaryInteractionRequiredAbilityTag(LookAtInteractableActor)
		                                  : IInteractable::Execute_GetSecondaryInteractionRequiredAbilityTag(LookAtInteractableActor);

	if (!RequiredAbilityTag.IsValid())
	{
		bIsPrimaryInteraction
			? IInteractable::Execute_PrimaryInteract(LookAtInteractableActor, Cast<AStealthCharacter>(GetOwner()))
			: IInteractable::Execute_SecondaryInteract(LookAtInteractableActor, Cast<AStealthCharacter>(GetOwner()));
	}
	else if (AbilitiesComponent)
	{
		FGameplayEventData EventData;
		EventData.Target = LookAtInteractableActor;
		AbilitiesComponent->HandleGameplayEvent(RequiredAbilityTag, &EventData);
	}

	FInteractionNotifyMessage InteractionMessage;
	InteractionMessage.InteractionTag = IInteractable::Execute_GetPrimaryInteractionMessageTag(LookAtInteractableActor);
	UGameplayMessageSubsystem::Get(this).BroadcastMessage(StealthMessageChannels::TAG_Message_InteractionPerformed.GetTag(), InteractionMessage);

	//TODO: Move adding to inventory to some other place
	if (bIsPrimaryInteraction && InventoryComponent && LookAtInteractableActor->Implements<UPickable>())
	{
		int32 QuantityAfterAdd = 0;
		InventoryComponent->TryAddItem(IPickable::Execute_GetInventoryItem(LookAtInteractableActor), QuantityAfterAdd, IPickable::Execute_GetQuantity(LookAtInteractableActor));
	}
}
