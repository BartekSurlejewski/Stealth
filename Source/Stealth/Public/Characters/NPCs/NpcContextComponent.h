#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/AI/NpcAiComponentBase.h"
#include "GameplayTagContainer.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "Characters/NPCs/AI/AiCrimeEventPayload.h"
#include "NpcContextComponent.generated.h"

class UNpcProfile;
class UStateTreeAIComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNpcStateChanged, const FGameplayTag&, NewStateTag, const FGameplayTag&, OldStateTag);

/**
 * Context Component holding high-level NPC state tags, stance tags, profile reference,
 * and dispatching StateTree events.
 */
UCLASS(Blueprintable, ClassGroup = "NPC")
class STEALTH_API UNpcContextComponent : public UNpcAiComponentBase
{
	GENERATED_BODY()

public:
	UNpcContextComponent();

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintAssignable, Category = "NPC|Events")
	FOnNpcStateChanged OnNpcStateChanged;

	// State Bookkeeping
	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void SetCurrentStateTag(const FGameplayTag& NewStateTag);

	UFUNCTION(BlueprintCallable, Category = "NPC|State")
	void SetCurrentStanceTag(const FGameplayTag& NewStanceTag);

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	FGameplayTag GetCurrentStateTag() const { return CurrentStateTag; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	FGameplayTag GetCurrentStanceTag() const { return CurrentStanceTag; }

	UFUNCTION(BlueprintPure, Category = "NPC|State")
	const FGuid& GetNpcGuid() const { return OwnerNpcGuid; }

	void SetNpcGuid(const FGuid& InGuid) { OwnerNpcGuid = InGuid; }

	// Profile Access
	virtual const UNpcProfile* GetNpcProfile() const override { return NpcProfile; }

	UFUNCTION(BlueprintCallable, Category = "NPC|Profile")
	void SetNpcProfile(UNpcProfile* InProfile) { NpcProfile = InProfile; }

	// StateTree Event Dispatch
	UFUNCTION(BlueprintCallable, Category = "NPC|StateTree")
	void SendStateTreeEvent(const FGameplayTag& EventTag);

	/** Handle crime report dispatched from HousingSubsystem */
	UFUNCTION(BlueprintCallable, Category = "NPC|Crime")
	virtual void HandleCrimeReported(const FAiCrimeEventPayload& CrimePayload);

protected:
	UStateTreeAIComponent* GetStateTreeComponent() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Profile")
	TObjectPtr<UNpcProfile> NpcProfile;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
	FGameplayTag CurrentStateTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
	FGameplayTag CurrentStanceTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Identification")
	FGuid OwnerNpcGuid;

private:
	mutable TWeakObjectPtr<UStateTreeAIComponent> CachedStateTreeComponent;
};
