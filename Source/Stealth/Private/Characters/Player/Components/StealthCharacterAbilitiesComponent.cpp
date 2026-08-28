#include "Characters/Player/Components/StealthCharacterAbilitiesComponent.h"

#include "AbilitySystemComponent.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "GameplayEffect.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Inventory/InventoryComponent.h"
#include "Inventory/Items/ItemDefinition.h"
#include "Messages/StealthMessages.h"


UStealthCharacterAbilitiesComponent::UStealthCharacterAbilitiesComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStealthCharacterAbilitiesComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningCharacter = Cast<AStealthPlayerCharacter>(GetOwner());
	if (OwningCharacter != nullptr)
	{
		AbilitySystemComponent = OwningCharacter->GetAbilitySystemComponent();
		AttributeSet = OwningCharacter->GetAttributeSet();
		PlayerInventoryComponent = OwningCharacter->GetPlayerState()->FindComponentByClass<UInventoryComponent>();
	}

	GrantAbilities(StartingAbilities);
	ApplyGameplayEffectsToSelf(StartingEffects);

	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	PlayerInventoryItemAddedHandle = MsgSubsystem.RegisterListener<FPlayerInventoryItemAddedMessage>(StealthMessageChannels::TAG_Message_Inventory_ItemAdded, this,
	                                                                                                 &UStealthCharacterAbilitiesComponent::PlayerInventory_OnItemAdded);
	PlayerInventoryItemRemovedHandle = MsgSubsystem.RegisterListener<FPlayerInventoryItemRemovedMessage>(StealthMessageChannels::TAG_Message_Inventory_ItemRemoved, this,
	                                                                                                     &UStealthCharacterAbilitiesComponent::PlayerInventory_OnItemRemoved);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->OnAbilityEnded.AddUObject(this, &UStealthCharacterAbilitiesComponent::AbilitySystemComponent_OnAbilityEnded);
	}
}

void UStealthCharacterAbilitiesComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(this);
	MsgSubsystem.UnregisterListener(PlayerInventoryItemAddedHandle);
	MsgSubsystem.UnregisterListener(PlayerInventoryItemRemovedHandle);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->OnAbilityEnded.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
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

bool UStealthCharacterAbilitiesComponent::TryActivateAbilitiesWithTag(const FGameplayTagContainer& TagContainer) const
{
	if (!AbilitySystemComponent)
	{
		return false;
	}

	return AbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);
}

void UStealthCharacterAbilitiesComponent::HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload) const
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->HandleGameplayEvent(EventTag, Payload);
	}
}

void UStealthCharacterAbilitiesComponent::PlayerInventory_OnItemAdded(FGameplayTag Channel, const FPlayerInventoryItemAddedMessage& Message)
{
	if (Message.AddedQuantity <= 0 || Message.QuantityInInventory <= 0 || OnAddItemsGrantedAbilities.Find(Message.AddedItem))
	{
		return;
	}

	OnAddItemsGrantedAbilities.Add(Message.AddedItem, FAbilityHandleList{GrantAbilities(Message.AddedItem->AbilitiesToGrantOnAdd)});
}

void UStealthCharacterAbilitiesComponent::PlayerInventory_OnItemRemoved(FGameplayTag Channel, const FPlayerInventoryItemRemovedMessage& Message)
{
	if (Message.RemovedQuantity <= 0 || Message.QuantityInInventory > 0 || !OnAddItemsGrantedAbilities.Find(Message.RemovedItem))
	{
		return;
	}

	RemoveAbilities(OnAddItemsGrantedAbilities[Message.RemovedItem].Handles);
	OnAddItemsGrantedAbilities.Remove(Message.RemovedItem);
}

void UStealthCharacterAbilitiesComponent::AbilitySystemComponent_OnAbilityEnded(const FAbilityEndedData& AbilityEndedData)
{
	OnAbilityEnded.Broadcast(AbilityEndedData);
}
