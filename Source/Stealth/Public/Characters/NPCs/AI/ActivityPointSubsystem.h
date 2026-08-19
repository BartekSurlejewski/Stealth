#pragma once

#include "CoreMinimal.h"
#include "Core/StealthWorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "ActivityPointSubsystem.generated.h"

class AActor;

UCLASS()
class STEALTH_API UActivityPointSubsystem : public UStealthWorldSubsystem
{
	GENERATED_BODY()

public:
	static UActivityPointSubsystem* Get(const UObject* WorldContextObject)
	{
		return UStealthWorldSubsystem::Get<UActivityPointSubsystem>(WorldContextObject);
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Register an actor as a POI / Activity Point with associated tags */
	UFUNCTION(BlueprintCallable, Category = "AI|Activity")
	void RegisterActivityPoint(AActor* ActivityActor, const FGameplayTagContainer& ActivityTags, int32 MaxOccupancy = 1);

	/** Unregister an activity point */
	UFUNCTION(BlueprintCallable, Category = "AI|Activity")
	void UnregisterActivityPoint(AActor* ActivityActor);

	/** Attempt to claim a suitable activity point matching the requested tag */
	UFUNCTION(BlueprintCallable, Category = "AI|Activity")
	AActor* ClaimActivityPoint(const FGuid& NpcGuid, const FGameplayTag& ActivityTag, const FVector& RequesterLocation = FVector::ZeroVector, AActor* PreferredActor = nullptr);

	/** Release any claimed activity point held by the NPC */
	UFUNCTION(BlueprintCallable, Category = "AI|Activity")
	void ReleaseActivityPoint(const FGuid& NpcGuid, AActor* SpecificActor = nullptr);

	/** Check if an activity point is currently claimed */
	UFUNCTION(BlueprintPure, Category = "AI|Activity")
	bool IsActivityPointOccupied(const AActor* ActivityActor) const;

	/** Get the current claim information for an actor */
	UFUNCTION(BlueprintPure, Category = "AI|Activity")
	bool GetClaimForNpc(const FGuid& NpcGuid, FActivitySlotClaim& OutClaim) const;

private:
	struct FActivityPointEntry
	{
		TWeakObjectPtr<AActor> Actor;
		FGameplayTagContainer Tags;
		int32 MaxOccupancy = 1;
		TArray<FGuid> CurrentClaimers;
	};

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> RegisteredActors;

	TMap<TWeakObjectPtr<AActor>, FActivityPointEntry> ActivityPoints;
	TMap<FGuid, FActivitySlotClaim> ActiveClaims;
};
