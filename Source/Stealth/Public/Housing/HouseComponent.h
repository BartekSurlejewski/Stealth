#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Housing/HouseData.h"
#include "HouseComponent.generated.h"

struct FHitResult;
class ANpcCharacter;
class UBoxComponent;
class ATriggerVolume;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHouseOccupantChanged, UHouseComponent*, House, AActor*, Actor);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHouseCrimeDetected, UHouseComponent*, House, const FHouseCrimeReport&, CrimeReport);

UCLASS(ClassGroup=(Housing), meta=(BlueprintSpawnableComponent))
class STEALTH_API UHouseComponent : public UActorComponent
{
	GENERATED_BODY()

	/*Events*/
public:
	UPROPERTY(BlueprintAssignable, Category = "Housing|Events")
	FOnHouseOccupantChanged OnActorEntered;

	UPROPERTY(BlueprintAssignable, Category = "Housing|Events")
	FOnHouseOccupantChanged OnActorExited;

	UPROPERTY(BlueprintAssignable, Category = "Housing|Events")
	FOnHouseCrimeDetected OnCrimeDetected;

public:
	UHouseComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/* Getters */
	[[nodiscard]] const FName& GetHouseId() const { return HouseId; }
	[[nodiscard]] const FText& GetHouseDisplayName() const { return HouseDisplayName; }
	[[nodiscard]] const TArray<FGuid>& GetOwnerGuids() const { return OwnerGuids; }
	[[nodiscard]] bool IsPlayerOwned() const { return bPlayerOwned; }
	[[nodiscard]] const TArray<TObjectPtr<AActor>>& GetDoors() const { return Doors; }
	[[nodiscard]] const TArray<TObjectPtr<AActor>>& GetContainers() const { return OwnedContainers; }
	[[nodiscard]] const TArray<TObjectPtr<AActor>>& GetCurrentOccupants() const { return CurrentOccupants; }
	[[nodiscard]] const FHouseActivityPoints& GetActivityPoints() const { return ActivityPoints; }

	/* Ownership & Permissions */
	UFUNCTION(BlueprintPure, Category = "Housing")
	EHouseAccessLevel GetActorAccessLevel(const AActor* Actor) const;
	UFUNCTION(BlueprintPure, Category = "Housing")
	bool IsActorAllowed(const AActor* Actor) const;
	UFUNCTION(BlueprintPure, Category = "Housing")
	bool IsOwner(const AActor* Actor) const;

	/* Routine Spots */
	UFUNCTION(BlueprintCallable, Category = "Housing|Routine")
	AActor* GetBedForNpc(const FGuid& NpcGuid) const;
	UFUNCTION(BlueprintCallable, Category = "Housing|Routine")
	void AssignBedToNpc(const FGuid& NpcGuid, AActor* BedActor);

	/* Boundary & Crime */
	UFUNCTION(BlueprintCallable, Category = "Housing|Crime")
	void NotifyCrime(EHouseCrimeType CrimeType, AActor* Perpetrator, AActor* TargetObject);
	UFUNCTION(BlueprintCallable, Category = "Housing")
	void SetHouseVolumeTrigger(UPrimitiveComponent* InVolumeTrigger);

protected:
	UFUNCTION()
	void HandleVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                              bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Housing")
	FName HouseId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Housing")
	FText HouseDisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Housing")
	bool bPlayerOwned = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Housing")
	bool bPublicDuringDay = false;

	/** Owner NPC GUIDs */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Housing")
	TArray<FGuid> OwnerGuids;

	/** Direct references to initial NPC owners (converted to GUIDs at BeginPlay) */
	UPROPERTY(EditAnywhere, Category = "Housing")
	TArray<TObjectPtr<ANpcCharacter>> InitialNpcOwners;

	/** Doors belonging to this house */
	UPROPERTY(EditAnywhere, Category = "Housing|Interactables")
	TArray<TObjectPtr<AActor>> Doors;

	/** Chests/containers owned by this house */
	UPROPERTY(EditAnywhere, Category = "Housing|Interactables")
	TArray<TObjectPtr<AActor>> OwnedContainers;

	/** Smart objects / points for NPC daily tasks */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Housing|Routine")
	FHouseActivityPoints ActivityPoints;

	/** Trigger component used for detecting when characters enter/leave */
	UPROPERTY(EditAnywhere, Category = "Housing|Bounds")
	TObjectPtr<UPrimitiveComponent> HouseVolumeTrigger;

	/** Actors currently inside the house */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Housing|State")
	TArray<TObjectPtr<AActor>> CurrentOccupants;
};
