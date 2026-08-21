#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Core/StealthWorldSubsystem.h"
#include "Housing/HouseData.h"
#include "HousingSubsystem.generated.h"

class UHouseComponent;
class ANpcCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHousingCrimeReported, const FHouseCrimeReport&, CrimeReport);

UCLASS()
class STEALTH_API UHousingSubsystem : public UStealthWorldSubsystem
{
	GENERATED_BODY()

public:
	static UHousingSubsystem* Get(const UObject* WorldContextObject)
	{
		return UStealthWorldSubsystem::Get<UHousingSubsystem>(WorldContextObject);
	}

	/* Registration & Lifecycle */
	void RegisterHouse(UHouseComponent* HouseComponent);
	void UnregisterHouse(UHouseComponent* HouseComponent);
	void RebuildOwnerIndexMap();

	/* Queries */
	UFUNCTION(BlueprintPure, Category = "Housing")
	UHouseComponent* GetHouseById(const FName& HouseId) const;

	UFUNCTION(BlueprintPure, Category = "Housing")
	UHouseComponent* GetHouseForNpc(const FGuid& NpcGuid) const;

	UFUNCTION(BlueprintPure, Category = "Housing")
	UHouseComponent* GetCurrentHouseForActor(const AActor* Actor) const;

	UFUNCTION(BlueprintPure, Category = "Housing")
	bool IsActorAllowedInHouse(const FName& HouseId, const AActor* Actor) const;

	UFUNCTION(BlueprintPure, Category = "Housing")
	bool IsActorTrespassing(const AActor* Actor) const;

	/* Routine Support */
	UFUNCTION(BlueprintPure, Category = "Housing|Routine")
	AActor* GetNpcAssignedBed(const FGuid& NpcGuid) const;

	UFUNCTION(BlueprintPure, Category = "Housing|Routine")
	FVector GetNpcHomeLocation(const FGuid& NpcGuid) const;

	/* Crime & Reactions */
	UFUNCTION(BlueprintCallable, Category = "Housing|Crime")
	void ReportCrime(const FHouseCrimeReport& CrimeReport) const;

	UFUNCTION(BlueprintCallable, Category = "Housing|Crime")
	void ReportIntrusion(const FName& HouseId, AActor* Intruder) const;

	/* Occupancy Tracking (called by HouseComponent) */
	void OnActorEnteredHouse(UHouseComponent* House, AActor* Actor);
	void OnActorLeftHouse(UHouseComponent* House, AActor* Actor);

public:
	UPROPERTY(BlueprintAssignable, Category = "Housing|Events")
	FOnHousingCrimeReported OnCrimeReported;

	/** Gameplay tag dispatched to owners/guards when an intrusion/theft happens */
	UPROPERTY(EditDefaultsOnly, Category = "Housing|Tags")
	FGameplayTag IntrusionStateTreeEventTag;

	UPROPERTY(EditDefaultsOnly, Category = "Housing|Tags")
	FGameplayTag TheftStateTreeEventTag;

private:
	UPROPERTY()
	TArray<TObjectPtr<UHouseComponent>> Houses;

	UPROPERTY()
	int32 PlayerHouseIndex;

	UPROPERTY()
	TMap<FName, int32> HouseIdToIndexMap;

	UPROPERTY()
	TMap<FGuid, int32> NpcOwnerToHouseIndexMap;

	UPROPERTY()
	TMap<TObjectPtr<const AActor>, TObjectPtr<UHouseComponent>> ActorOccupancyMap;
};
