#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayAbilitySpecHandle.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Inventory/Items/InventoryItem.h"
#include "StealthCharacterAbilitiesComponent.generated.h"

class UInventoryComponent;
class UGameplayEffect;
class UGameplayAbility;
class UStealthCharacterAttributeSet;
class AStealthCharacter;
class UAbilitySystemComponent;

// Used for map storage
USTRUCT()
struct FAbilityHandleList
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> Handles;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilityEnded, const FAbilityEndedData&);

// Handles the GAS abilities connected to StealthCharacter and the character status
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthCharacterAbilitiesComponent : public UActorComponent
{
	GENERATED_BODY()

	/*Events*/
public:
	FOnAbilityEnded OnAbilityEnded;

private:
	UPROPERTY()
	TObjectPtr<AStealthCharacter> OwningCharacter;
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	UPROPERTY()
	TObjectPtr<UStealthCharacterAttributeSet> AttributeSet;
	UPROPERTY()
	TObjectPtr<UInventoryComponent> PlayerInventoryComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
	FGameplayTag SprintAbilityTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
	FGameplayTag CrouchAbilityTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
	FGameplayTag JumpAbilityTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
	FGameplayTag IsMovingTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
	FGameplayTag StaminaRegenTag;
	UPROPERTY(EditDefaultsOnly, Category = "Tags", meta = (AllowPrivateAccess = "true"))
	FGameplayTag IsFallingTag;

	UPROPERTY()
	FGameplayTagContainer SprintAbilityTagsContainer;
	UPROPERTY()
	FGameplayTagContainer CrouchAbilityTagsContainer;
	UPROPERTY()
	FGameplayTagContainer JumpAbilityTagsContainer;

	UPROPERTY()
	TMap<TObjectPtr<UItemDefinition>, FAbilityHandleList> OnAddItemsGrantedAbilities;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability System", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UGameplayAbility>> StartingAbilities;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability System", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<UGameplayEffect>> StartingEffects;

public:
	UStealthCharacterAbilitiesComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category="Ability System")
	TArray<FGameplayAbilitySpecHandle> GrantAbilities(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToGrant);
	UFUNCTION(BlueprintCallable, Category="Ability System")
	void RemoveAbilities(TArray<FGameplayAbilitySpecHandle> AbilityHandlesToRemove);
	UFUNCTION(BlueprintCallable, Category="Ability System")
	TArray<FActiveGameplayEffectHandle> ApplyGameplayEffectsToSelf(TArray<TSubclassOf<UGameplayEffect>> EffectsToApply);
	UFUNCTION(BlueprintCallable, Category="Ability System")
	void RemoveGameplayEffects(TArray<FActiveGameplayEffectHandle> EffectHandlesToRemove);
	UFUNCTION(BlueprintCallable, Category="Ability System")
	bool TryActivateAbilitiesWithTag(const FGameplayTagContainer& TagContainer) const;
	// UFUNCTION(BlueprintCallable, Category="Ability System")
	void HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload) const;

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintInputStart();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintInputEnd();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchInputStart();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoCrouchInputEnd();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

private:
	UFUNCTION()
	void EndSprint() const;
	UFUNCTION()
	void UpdateTags() const;

	UFUNCTION()
	void InventoryComponent_OnItemAdded(FInventoryItem& Item, int AddedQuantity, int QuantityInInventory);
	UFUNCTION()
	void InventoryComponent_OnItemRemoved(FInventoryItem& Item, int RemovedQuantity, int QuantityInInventory, bool bShouldDrop);
	UFUNCTION()
	void AbilitySystemComponent_OnAbilityEnded(const FAbilityEndedData& AbilityEndedData);
};
