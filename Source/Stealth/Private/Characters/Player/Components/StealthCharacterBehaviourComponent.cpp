#include "Characters/Player/Components/StealthCharacterBehaviourComponent.h"

#include "AbilitySystemComponent.h"
#include "Characters/AttributeSets/StealthCharacterAttibuteSet.h"
#include "Characters/Player/StealthCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


UStealthCharacterBehaviourComponent::UStealthCharacterBehaviourComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStealthCharacterBehaviourComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<AStealthCharacter>(GetOwner());
	if (OwningCharacter != nullptr)
	{
		AbilitySystemComponent = OwningCharacter->GetAbilitySystemComponent();
		AttributeSet = OwningCharacter->GetAttributeSet();
	}

	SprintAbilityTagsContainer.AddTag(SprintAbilityTag);
	CrouchAbilityTagsContainer.AddTag(CrouchAbilityTag);
	JumpAbilityTagsContainer.AddTag(JumpAbilityTag);
}

void UStealthCharacterBehaviourComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTags();
	if (AttributeSet->GetStamina() <= 0)
	{
		EndSprint();
	}
}

void UStealthCharacterBehaviourComponent::DoSprintInputStart()
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->TryActivateAbilitiesByTag(SprintAbilityTagsContainer);
}

void UStealthCharacterBehaviourComponent::DoSprintInputEnd()
{
	EndSprint();
}

void UStealthCharacterBehaviourComponent::DoCrouchInputStart()
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->TryActivateAbilitiesByTag(CrouchAbilityTagsContainer);
}

void UStealthCharacterBehaviourComponent::DoCrouchInputEnd()
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->CancelAbilities(&CrouchAbilityTagsContainer);
}

void UStealthCharacterBehaviourComponent::DoJumpStart()
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->TryActivateAbilitiesByTag(JumpAbilityTagsContainer);
}

void UStealthCharacterBehaviourComponent::DoJumpEnd()
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->CancelAbilities(&JumpAbilityTagsContainer);
}

void UStealthCharacterBehaviourComponent::EndSprint() const
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->CancelAbilities(&SprintAbilityTagsContainer);
	AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(SprintAbilityTagsContainer);
}

void UStealthCharacterBehaviourComponent::UpdateTags() const
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	bool isFalling = OwningCharacter->GetCharacterMovement()->IsFalling();
	bool bIsMoving = OwningCharacter->GetVelocity().SizeSquared() > 100.f;
	bool bIsSprinting = AbilitySystemComponent->HasMatchingGameplayTag(SprintAbilityTag);

	bool bHasMovingTag = AbilitySystemComponent->HasMatchingGameplayTag(IsMovingTag);
	bool bHasRegenStaminaTag = AbilitySystemComponent->HasMatchingGameplayTag(StaminaRegenTag);
	bool bHasFallingStaminaTag = AbilitySystemComponent->HasMatchingGameplayTag(IsFallingTag);

	// Only update when state changes — avoid spamming ASC every frame
	if (isFalling && !bHasFallingStaminaTag)
	{
		AbilitySystemComponent->AddLooseGameplayTag(IsFallingTag);
	}
	else if (!isFalling && bHasFallingStaminaTag)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(IsFallingTag);
	}

	if (bIsMoving && !bHasMovingTag)
	{
		AbilitySystemComponent->AddLooseGameplayTag(IsMovingTag);
	}
	else if (!bIsMoving && bHasMovingTag)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(IsMovingTag);
	}

	bool bShouldRegenStamina = !bIsSprinting || !bIsMoving;
	if (bShouldRegenStamina && !bHasRegenStaminaTag)
	{
		AbilitySystemComponent->AddLooseGameplayTag(StaminaRegenTag);
	}
	else if (!bShouldRegenStamina && bHasRegenStaminaTag)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(StaminaRegenTag);
	}
}
