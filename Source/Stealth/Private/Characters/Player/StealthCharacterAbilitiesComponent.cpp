#include "Characters/Player/StealthCharacterAbilitiesComponent.h"

#include "AbilitySystemComponent.h"
#include "Characters/AttributeSets/StealthCharacterAttibuteSet.h"
#include "Characters/Player/StealthCharacter.h"


UStealthCharacterAbilitiesComponent::UStealthCharacterAbilitiesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStealthCharacterAbilitiesComponent::BeginPlay()
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
}

void UStealthCharacterAbilitiesComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTags();
	if (AttributeSet->GetStamina() <= 0)
	{
		EndSprint();
	}
}

void UStealthCharacterAbilitiesComponent::DoSprintInputStart()
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->TryActivateAbilitiesByTag(SprintAbilityTagsContainer);
}

void UStealthCharacterAbilitiesComponent::DoSprintInputEnd()
{
	EndSprint();
}

void UStealthCharacterAbilitiesComponent::DoCrouchInputStart()
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->TryActivateAbilitiesByTag(CrouchAbilityTagsContainer);
}

void UStealthCharacterAbilitiesComponent::DoCrouchInputEnd()
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->CancelAbilities(&CrouchAbilityTagsContainer);
}

void UStealthCharacterAbilitiesComponent::EndSprint() const
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	AbilitySystemComponent->CancelAbilities(&SprintAbilityTagsContainer);
	AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(SprintAbilityTagsContainer);
}

void UStealthCharacterAbilitiesComponent::UpdateTags() const
{
	if (AbilitySystemComponent == nullptr)
	{
		return;
	}

	bool bIsMoving = OwningCharacter->GetVelocity().SizeSquared() > 100.f;
	bool bIsSprinting = AbilitySystemComponent->HasMatchingGameplayTag(SprintAbilityTag);

	bool bHasMovingTag = AbilitySystemComponent->HasMatchingGameplayTag(MovingTag);
	bool bHasRegenStaminaTag = AbilitySystemComponent->HasMatchingGameplayTag(StaminaRegenTag);

	// Only update when state changes — avoid spamming ASC every frame
	if (bIsMoving && !bHasMovingTag)
	{
		AbilitySystemComponent->AddLooseGameplayTag(MovingTag);
	}
	else if (!bIsMoving && bHasMovingTag)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(MovingTag);
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
