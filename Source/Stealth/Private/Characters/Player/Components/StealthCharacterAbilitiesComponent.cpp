#include "Characters/Player/Components/StealthCharacterAbilitiesComponent.h"

#include "AbilitySystemComponent.h"
#include "Characters/AttributeSets/StealthCharacterAttibuteSet.h"
#include "Characters/Player/StealthCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffect.h"


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
	JumpAbilityTagsContainer.AddTag(JumpAbilityTag);

	GrantAbilities(StartingAbilities);
	ApplyGameplayEffectsToSelf(StartingEffects);
}

void UStealthCharacterAbilitiesComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateTags();
	if (AttributeSet && AttributeSet->GetStamina() <= 0)
	{
		EndSprint();
	}
}

TArray<FGameplayAbilitySpecHandle> UStealthCharacterAbilitiesComponent::GrantAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant)
{
	if (!AbilitySystemComponent)
	{
		return TArray<FGameplayAbilitySpecHandle>();
	}

	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

	for (const TSubclassOf<UGameplayAbility> Ability : AbilitiesToGrant)
	{
		if (!Ability)
		{
			continue;
		}
		FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1, -1, OwningCharacter));
		GrantedAbilityHandles.Add(SpecHandle);
	}

	return GrantedAbilityHandles;
}

void UStealthCharacterAbilitiesComponent::RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& Handle : AbilityHandlesToRemove)
	{
		AbilitySystemComponent->ClearAbility(Handle);
	}
}

TArray<FActiveGameplayEffectHandle> UStealthCharacterAbilitiesComponent::ApplyGameplayEffectsToSelf(TArray<TSubclassOf<UGameplayEffect>> EffectsToApply)
{
	if (!AbilitySystemComponent)
	{
		return TArray<FActiveGameplayEffectHandle>();
	}

	FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
	ContextHandle.AddSourceObject(OwningCharacter);

	TArray<FActiveGameplayEffectHandle> AppliedHandles;

	for (const TSubclassOf<UGameplayEffect>& EffectClass : EffectsToApply)
	{
		if (!EffectClass)
		{
			continue;
		}

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1, ContextHandle);

		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			AppliedHandles.Add(ActiveHandle);
		}
	}

	return AppliedHandles;
}

void UStealthCharacterAbilitiesComponent::RemoveGameplayEffects(TArray<FActiveGameplayEffectHandle> EffectHandlesToRemove)
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	for (const FActiveGameplayEffectHandle& Handle : EffectHandlesToRemove)
	{
		if (Handle.IsValid())
		{
			AbilitySystemComponent->RemoveActiveGameplayEffect(Handle);
		}
	}
}

void UStealthCharacterAbilitiesComponent::DoSprintInputStart()
{
	if (!AbilitySystemComponent)
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
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->TryActivateAbilitiesByTag(CrouchAbilityTagsContainer);
}

void UStealthCharacterAbilitiesComponent::DoCrouchInputEnd()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->CancelAbilities(&CrouchAbilityTagsContainer);
}

void UStealthCharacterAbilitiesComponent::DoJumpStart()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->TryActivateAbilitiesByTag(JumpAbilityTagsContainer);
}

void UStealthCharacterAbilitiesComponent::DoJumpEnd()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->CancelAbilities(&JumpAbilityTagsContainer);
}

void UStealthCharacterAbilitiesComponent::EndSprint() const
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	AbilitySystemComponent->CancelAbilities(&SprintAbilityTagsContainer);
	AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(SprintAbilityTagsContainer);
}

void UStealthCharacterAbilitiesComponent::UpdateTags() const
{
	if (!AbilitySystemComponent)
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
