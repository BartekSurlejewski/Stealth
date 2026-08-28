#include "Characters/Player/Components/PlayerMovementAbilityComponent.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "Characters/AttributeSets/BasicAttributeSet.h"
#include "Characters/Player/Movement/PlayerMovementAbilitySet.h"
#include "Characters/Player/StealthPlayerTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayerMovementAbilityComponent::UPlayerMovementAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UPlayerMovementAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<ACharacter>(GetOwner());
	if (OwningCharacter)
	{
		CharacterMovementComponent = OwningCharacter->GetCharacterMovement();
		AbilitySystemComponent = OwningCharacter->FindComponentByClass<UAbilitySystemComponent>();
	}

	if (AbilitySystemComponent)
	{
		if (MovementAbilitySet)
		{
			for (const FMovementAbilityBinding& Binding : MovementAbilitySet->AbilityBindings)
			{
				if (Binding.AbilityClass && Binding.AbilityTag.IsValid())
				{
					FGameplayAbilitySpec Spec(Binding.AbilityClass, 1, INDEX_NONE, OwningCharacter);
					FGameplayAbilitySpecHandle Handle = AbilitySystemComponent->GiveAbility(Spec);
					AbilitySpecHandles.Add(Binding.AbilityTag, Handle);
				}
			}
		}

		SpeedAttributeChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UBasicAttributeSet::GetSpeedCoefficientAttribute()).AddUObject(this, &UPlayerMovementAbilityComponent::OnSpeedAttributeChanged);

		if (const UBasicAttributeSet* AttributeSet = AbilitySystemComponent->GetSet<UBasicAttributeSet>())
		{
			if (CharacterMovementComponent)
			{
				CharacterMovementComponent->MaxWalkSpeed = AttributeSet->GetSpeedCoefficient() * AttributeSet->GetBaseCharacterSpeed();
			}
		}
	}
}

void UPlayerMovementAbilityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AbilitySystemComponent && SpeedAttributeChangedDelegateHandle.IsValid())
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			UBasicAttributeSet::GetSpeedCoefficientAttribute()).Remove(SpeedAttributeChangedDelegateHandle);
		SpeedAttributeChangedDelegateHandle.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void UPlayerMovementAbilityComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!AbilitySystemComponent || !CharacterMovementComponent)
	{
		return;
	}

	const bool bIsFalling = CharacterMovementComponent->IsFalling();
	if (bIsFalling != bWasFalling)
	{
		bWasFalling = bIsFalling;
		if (bIsFalling)
		{
			AbilitySystemComponent->AddLooseGameplayTag(StealthPlayerTags::TAG_Player_State_Movement_Falling);
		}
		else
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(StealthPlayerTags::TAG_Player_State_Movement_Falling);
		}
	}

	const bool bIsMoving = CharacterMovementComponent->Velocity.SizeSquared() > 100.0f;
	if (bIsMoving != bWasMoving)
	{
		bWasMoving = bIsMoving;
		if (bIsMoving)
		{
			AbilitySystemComponent->AddLooseGameplayTag(StealthPlayerTags::TAG_Player_State_Movement_Moving);
		}
		else
		{
			AbilitySystemComponent->RemoveLooseGameplayTag(StealthPlayerTags::TAG_Player_State_Movement_Moving);
		}
	}
}

void UPlayerMovementAbilityComponent::BindInput(UEnhancedInputComponent* EIC)
{
	if (!EIC || !MovementAbilitySet)
	{
		return;
	}

	for (const FMovementAbilityBinding& Binding : MovementAbilitySet->AbilityBindings)
	{
		if (!Binding.AbilityTag.IsValid())
		{
			continue;
		}

		switch (Binding.ActivationType)
		{
		case EMovementAbilityInputMode::Hold:
			if (Binding.InputAction)
			{
				EIC->BindAction(Binding.InputAction, ETriggerEvent::Started, this, &UPlayerMovementAbilityComponent::HandleAbilityPressed, Binding.AbilityTag);
				EIC->BindAction(Binding.InputAction, ETriggerEvent::Completed, this, &UPlayerMovementAbilityComponent::HandleAbilityReleased, Binding.AbilityTag);
			}
			break;

		case EMovementAbilityInputMode::Toggle:
			if (Binding.InputAction)
			{
				EIC->BindAction(Binding.InputAction, ETriggerEvent::Started, this, &UPlayerMovementAbilityComponent::HandleAbilityToggled, Binding.AbilityTag);
			}
			break;

		case EMovementAbilityInputMode::TwoActions:
			if (Binding.InputAction)
			{
				EIC->BindAction(Binding.InputAction, ETriggerEvent::Started, this, &UPlayerMovementAbilityComponent::HandleAbilityPressed, Binding.AbilityTag);
			}
			if (Binding.InputActionDisable)
			{
				EIC->BindAction(Binding.InputActionDisable, ETriggerEvent::Started, this, &UPlayerMovementAbilityComponent::HandleAbilityReleased, Binding.AbilityTag);
			}
			break;
		}
	}
}

void UPlayerMovementAbilityComponent::HandleAbilityPressed(FGameplayTag AbilityTag)
{
	if (!AbilitySystemComponent || !AbilityTag.IsValid())
	{
		return;
	}

	const FGameplayTagContainer TagContainer(AbilityTag);
	AbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);
}

void UPlayerMovementAbilityComponent::HandleAbilityReleased(FGameplayTag AbilityTag)
{
	if (!AbilitySystemComponent || !AbilityTag.IsValid())
	{
		return;
	}

	if (const FGameplayAbilitySpecHandle* SpecHandle = AbilitySpecHandles.Find(AbilityTag))
	{
		AbilitySystemComponent->CancelAbilityHandle(*SpecHandle);
	}
}

void UPlayerMovementAbilityComponent::HandleAbilityToggled(FGameplayTag AbilityTag)
{
	if (!AbilitySystemComponent || !AbilityTag.IsValid())
	{
		return;
	}

	if (const FGameplayAbilitySpecHandle* SpecHandle = AbilitySpecHandles.Find(AbilityTag))
	{
		if (const FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(*SpecHandle))
		{
			if (Spec->IsActive())
			{
				HandleAbilityReleased(AbilityTag);
			}
			else
			{
				HandleAbilityPressed(AbilityTag);
			}
		}
	}
}

void UPlayerMovementAbilityComponent::OnSpeedAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (CharacterMovementComponent && AbilitySystemComponent)
	{
		if (const UBasicAttributeSet* AttributeSet = AbilitySystemComponent->GetSet<UBasicAttributeSet>())
		{
			CharacterMovementComponent->MaxWalkSpeed = Data.NewValue * AttributeSet->GetBaseCharacterSpeed();
		}
	}
}
