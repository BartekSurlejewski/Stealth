#pragma once

#include "CoreMinimal.h"
#include "NpcCharacter.h"
#include "Tickable.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CharactersRegistrySubsystem.generated.h"

class AStealthPlayerCharacter;
class ANpcAiController;
class UNpcContextComponent;
class UInventoryComponent;

USTRUCT(BlueprintType)
struct FNpcRegistryEntry
{
	GENERATED_BODY()

	// Primary references
	UPROPERTY(BlueprintReadOnly, Category="Registry|NPC")
	TObjectPtr<ANpcCharacter> Character;

	UPROPERTY(BlueprintReadOnly, Category="Registry|NPC")
	TObjectPtr<ANpcAiController> Controller;

	// Cached component references (O(1) access)
	UPROPERTY(BlueprintReadOnly, Category="Registry|NPC")
	TObjectPtr<UInventoryComponent> Inventory;

	UPROPERTY(BlueprintReadOnly, Category="Registry|NPC")
	TObjectPtr<UNpcContextComponent> ContextComponent;

	// Metadata
	UPROPERTY(BlueprintReadOnly, Category="Registry|NPC")
	FString NpcName;

	UPROPERTY(BlueprintReadOnly, Category="Registry|NPC")
	FVector LastKnownLocation;

	// Validation
	bool IsValid() const
	{
		return Character != nullptr && Controller != nullptr;
	}

	bool IsAlive() const
	{
		return IsValid() && !Character->IsActorBeingDestroyed();
	}

	void UpdateLocation()
	{
		if (Character)
		{
			LastKnownLocation = Character->GetActorLocation();
		}
	}
};

UCLASS()
class STEALTH_API UCharactersRegistrySubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

	/*=========================================================================
	 * LIFECYCLE
	 *=========================================================================*/
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="NPC Registry", meta=(WorldContext="WorldContextObject"))
	static UCharactersRegistrySubsystem* Get(const UObject* WorldContextObject);

	/*=========================================================================
	 * TICKABLE
	 *=========================================================================*/
public:
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override;
	virtual bool IsTickableWhenPaused() const override { return false; }

private:
	UPROPERTY()
	float TimeSinceLastTick = 0.0f;
	static constexpr float TickInterval = 0.2f;

	/*=========================================================================
	 * REGISTRATION / UNREGISTRATION
	 *=========================================================================*/
public:
	UFUNCTION(Category="Registry|Player")
	void RegisterPlayerCharacter(AStealthPlayerCharacter* NewPlayerCharacter);
	UFUNCTION(Category="Registry|Player")
	void UnregisterPlayerCharacter(AStealthPlayerCharacter* PlayerCharacterToRemove);

	/**
	 * Register an NPC when it spawns
	 * Call from ANpcAiController::OnPossess()
	 */
	UFUNCTION(Category="Registry|NPC")
	void RegisterNpc(ANpcCharacter* Character, ANpcAiController* Controller);

	/**
	 * Unregister when NPC is destroyed
	 * Call from ANpcAiController::EndPlay()
	 */
	UFUNCTION(Category="Registry|NPC")
	void UnregisterNpc(const FGuid& NpcGUID);

	/**
	 * Manual unregister by character (convenience)
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	void UnregisterNpcByCharacter(ANpcCharacter* Character);

	/*=========================================================================
	 * LOOKUPS: By GUID (Primary Key)
	 *=========================================================================*/
public:
	UFUNCTION(BlueprintCallable, Category="Registry|Player")
	AStealthPlayerCharacter* GetPlayerCharacter() const;
	
	/**
	 * Get complete NPC entry (O(1))
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	bool TryGetNpcEntry(const FGuid& NpcGUID, FNpcRegistryEntry& OutEntry) const;

	/**
	 * Get NPC character by GUID (O(1))
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	ANpcCharacter* GetNpcCharacter(const FGuid& NpcGUID) const;

	/**
	 * Get NPC AI controller by GUID (O(1))
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	ANpcAiController* GetNpcController(const FGuid& NpcGUID) const;

	/**
	 * Get NPC inventory by GUID (O(1))
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	UInventoryComponent* GetNpcInventory(const FGuid& NpcGUID) const;

	/**
	 * Get NPC context component by GUID (O(1))
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	UNpcContextComponent* GetNpcContextComponent(const FGuid& NpcGUID) const;

	/*=========================================================================
	 * LOOKUPS: By Character (Reverse Lookup)
	 *=========================================================================*/
public:

	/**
	 * Get GUID from character (O(1) via cached map)
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	FGuid GetNpcGuidByCharacter(ANpcCharacter* Character) const;

	/**
	 * Get GUID from controller (O(1))
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	FGuid GetNpcGuidByController(ANpcAiController* Controller) const;

	/**
	 * Get GUID from inventory (O(1) via cached map)
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	FGuid GetNpcGuidByInventory(UInventoryComponent* Inventory) const;

	/*=========================================================================
	 * QUERIES: Find by Filter
	 *=========================================================================*/
public:
	/**
	 * Get NPCs within radius (useful for alerts, proximity checks)
	 * Uses cached LastKnownLocation via spatial grid — call UpdateNpcLocation/UpdateAllNpcLocations
	 * beforehand if you need this to reflect movement since the last update.
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	void GetNpcsInRadius(const FVector& CenterLocation, float Radius, TArray<FGuid>& OutGuids) const;

	/**
	 * Get NPCs by type (Guard, Prisoner, etc.)
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	void GetNpcsByClass(TSubclassOf<ANpcCharacter> NpcClass, TArray<FGuid>& OutGuids) const;

	/**
	 * Get alive NPCs only
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	void GetAliveNpcs(TArray<FGuid>& OutGuids) const;

	/**
	 * Find closest NPC to location
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	FGuid FindClosestNpc(const FVector& Location, float MaxDistance = 10000.0f) const;

	/**
	 * Get NPC count
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	int32 GetNpcCount() const { return NpcRegistry.Num(); }

	// Reverse lookup: Controller → GUID (O(1))
private:
	UPROPERTY()
	TMap<TObjectPtr<ANpcAiController>, FGuid> ControllerToGuidMap;

	/*=========================================================================
	 * SPATIAL GRID (uniform grid hash for radius queries)
	 *=========================================================================*/

	static constexpr float SpatialCellSize = 500.0f; // cm; tune to typical query radius

	// Cell (X,Y in grid space) → GUIDs currently in that cell. Grid is 2D (Z ignored),
	// fine for a stealth game where verticality within a query radius is usually small;
	// widen the Z tolerance in the distance check below if that's not true for you.
	TMap<FIntPoint, TArray<FGuid>> SpatialGrid;

	// Which cell each GUID currently occupies, so moves/removals are O(1) instead of a scan.
	TMap<FGuid, FIntPoint> GuidToCellMap;

	FIntPoint LocationToCell(const FVector& Location) const;
	void AddToSpatialGrid(const FGuid& Guid, const FVector& Location);
	void RemoveFromSpatialGrid(const FGuid& Guid);
	void UpdateSpatialGridEntry(const FGuid& Guid, const FVector& NewLocation);

	/*=========================================================================
	 * BATCH OPERATIONS
	 *=========================================================================*/
public:
	/**
	 * Call function on all NPCs (useful for game state changes)
	 */
	template <typename Func>
	void ForEachNpc(Func InFunction) const
	{
		for (const auto& [Guid, Entry] : NpcRegistry)
		{
			if (Entry.IsAlive())
			{
				InFunction(Guid, Entry);
			}
		}
	}

	/**
	 * Send event to all context components
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	void BroadcastEventToAllNpcs(const FGameplayTag& EventTag);

protected:
	/**
	 * Update cached locations for all NPCs
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	void UpdateAllNpcLocations();

	/*=========================================================================
	 * UTILITY
	 *=========================================================================*/
public:
	/**
	 * Check if NPC GUID is registered and alive
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	bool IsNpcAlive(const FGuid& NpcGUID) const;

	/**
	 * Get registry stats (for debugging/monitoring)
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	FString GetRegistryStats() const;

	/**
	 * Clear all entries (called on map transition)
	 */
	UFUNCTION(BlueprintCallable, Category="Registry|NPC")
	void ClearRegistry();

	/*=========================================================================
	 * DELEGATES: For systems that need to react to NPC lifecycle
	 *=========================================================================*/
public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNpcRegistered, const FGuid&, NpcGUID, ANpcCharacter*, Character);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNpcUnregistered, const FGuid&, NpcGUID, ANpcCharacter*, Character);

	UPROPERTY(BlueprintAssignable, Category="NPC|Events")
	FOnNpcRegistered OnNpcRegistered;

	UPROPERTY(BlueprintAssignable, Category="NPC|Events")
	FOnNpcUnregistered OnNpcUnregistered;

	/*=========================================================================
	 * PRIVATE
	 *=========================================================================*/
private:
	UPROPERTY()
	TObjectPtr<AStealthPlayerCharacter> PlayerCharacter;

	// Primary storage: GUID → Entry
	UPROPERTY()
	TMap<FGuid, FNpcRegistryEntry> NpcRegistry;

	// Reverse lookup: Character → GUID (for O(1) reverse lookup)
	UPROPERTY()
	TMap<TObjectPtr<ANpcCharacter>, FGuid> CharacterToGuidMap;

	// Reverse lookup: Inventory → GUID
	UPROPERTY()
	TMap<TObjectPtr<UInventoryComponent>, FGuid> InventoryToGuidMap;
};
