#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcCharacter.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Inventory/InventoryComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UCharactersRegistrySubsystem* UCharactersRegistrySubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
		return nullptr;

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}
	const UGameInstance* GameInstance = World->GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UCharactersRegistrySubsystem>() : nullptr;
}

/*=========================================================================
 * LIFECYCLE
 *=========================================================================*/

void UCharactersRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("[NpcRegistry] Subsystem initialized"));
}

void UCharactersRegistrySubsystem::Deinitialize()
{
	ClearRegistry();
	Super::Deinitialize();

	UE_LOG(LogTemp, Warning, TEXT("[NpcRegistry] Subsystem deinitialized"));
}

/*=========================================================================
 * TICKABLE
 *=========================================================================*/
void UCharactersRegistrySubsystem::Tick(float DeltaTime)
{
	TimeSinceLastTick += DeltaTime;
	if (TimeSinceLastTick < TickInterval)
	{
		return;
	}

	TimeSinceLastTick = 0.0f;
	UpdateAllNpcLocations();
}

TStatId UCharactersRegistrySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UNpcRegistrySubsystem, STATGROUP_Tickables);
}


/*=========================================================================
 * REGISTRATION
 *=========================================================================*/

void UCharactersRegistrySubsystem::RegisterPlayerCharacter(AStealthPlayerCharacter* NewPlayerCharacter)
{
	if (NewPlayerCharacter != PlayerCharacter)
	{
		PlayerCharacter = NewPlayerCharacter;
		if (PlayerCharacter)
		{
			UE_LOG(LogTemp, Warning, TEXT("[NpcRegistry] Registered player character: %s"), *PlayerCharacter->GetName());
		}
	}
}

void UCharactersRegistrySubsystem::UnregisterPlayerCharacter(AStealthPlayerCharacter* PlayerCharacterToRemove)
{
	if (PlayerCharacterToRemove == PlayerCharacter)
	{
		PlayerCharacter = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("[NpcRegistry] Unregistered player character"));
	}
}

void UCharactersRegistrySubsystem::RegisterNpc(ANpcCharacter* Character, ANpcAiController* Controller)
{
	if (!Character || !Controller)
	{
		UE_LOG(LogTemp, Error, TEXT("[NpcRegistry] Invalid character or controller"));
		return;
	}

	const FGuid& NpcGUID = Character->GetNpcGUID();
	if (!NpcGUID.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[NpcRegistry] NPC has invalid GUID"));
		return;
	}

	// Check if already registered
	if (NpcRegistry.Contains(NpcGUID))
	{
		UE_LOG(LogTemp, Warning, TEXT("[NpcRegistry] NPC %s already registered"), *NpcGUID.ToString());
		return;
	}

	// Build entry
	FNpcRegistryEntry Entry;
	Entry.Character = Character;
	Entry.Controller = Controller;
	Entry.NpcName = Character->GetName();
	Entry.UpdateLocation();

	// Cache component references
	Entry.Inventory = Controller->FindComponentByClass<UInventoryComponent>();
	if (!Entry.Inventory)
	{
		Entry.Inventory = Controller->FindComponentByClass<UInventoryComponent>();
	}

	Entry.ContextComponent = Character->FindComponentByClass<UNpcContextComponent>();
	if (!Entry.ContextComponent)
	{
		Entry.ContextComponent = Controller->FindComponentByClass<UNpcContextComponent>();
	}

	// Store in primary registry
	NpcRegistry.Add(NpcGUID, Entry);

	// Store in reverse lookups
	CharacterToGuidMap.Add(Character, NpcGUID);
	ControllerToGuidMap.Add(Controller, NpcGUID);
	if (Entry.Inventory)
	{
		InventoryToGuidMap.Add(Entry.Inventory, NpcGUID);
	}

	AddToSpatialGrid(NpcGUID, Entry.LastKnownLocation);

	UE_LOG(LogTemp, Warning, TEXT("[NpcRegistry] Registered NPC '%s' (GUID: %s)"),
	       *Character->GetName(), *NpcGUID.ToString());

	// Broadcast event
	OnNpcRegistered.Broadcast(NpcGUID, Character);
}

void UCharactersRegistrySubsystem::UnregisterNpc(const FGuid& NpcGUID)
{
	const FNpcRegistryEntry* Entry = NpcRegistry.Find(NpcGUID);
	if (!Entry)
	{
		UE_LOG(LogTemp, Warning, TEXT("[NpcRegistry] NPC not found: %s"), *NpcGUID.ToString());
		return;
	}

	ANpcCharacter* Character = Entry->Character;

	// Remove from all maps
	NpcRegistry.Remove(NpcGUID);
	CharacterToGuidMap.Remove(Character);
	ControllerToGuidMap.Remove(Entry->Controller);
	if (Entry->Inventory)
	{
		InventoryToGuidMap.Remove(Entry->Inventory);
	}
	RemoveFromSpatialGrid(NpcGUID);

	UE_LOG(LogTemp, Warning, TEXT("[NpcRegistry] Unregistered NPC '%s'"),
	       *Character->GetName());

	// Broadcast event
	OnNpcUnregistered.Broadcast(NpcGUID, Character);
}

void UCharactersRegistrySubsystem::UnregisterNpcByCharacter(ANpcCharacter* Character)
{
	if (!Character)
		return;

	const FGuid* GuidPtr = CharacterToGuidMap.Find(Character);
	if (GuidPtr)
	{
		UnregisterNpc(*GuidPtr);
	}
}

/*=========================================================================
 * LOOKUPS: By GUID
 *=========================================================================*/

AStealthPlayerCharacter* UCharactersRegistrySubsystem::GetPlayerCharacter() const
{
	return PlayerCharacter;
}


bool UCharactersRegistrySubsystem::TryGetNpcEntry(const FGuid& NpcGUID, FNpcRegistryEntry& OutEntry) const
{
	if (const FNpcRegistryEntry* Entry = NpcRegistry.Find(NpcGUID))
	{
		OutEntry = *Entry;
		return true;
	}
	return false;
}

ANpcCharacter* UCharactersRegistrySubsystem::GetNpcCharacter(const FGuid& NpcGUID) const
{
	const FNpcRegistryEntry* Entry = NpcRegistry.Find(NpcGUID);
	return Entry ? Entry->Character : nullptr;
}

ANpcAiController* UCharactersRegistrySubsystem::GetNpcController(const FGuid& NpcGUID) const
{
	const FNpcRegistryEntry* Entry = NpcRegistry.Find(NpcGUID);
	return Entry ? Entry->Controller : nullptr;
}

UInventoryComponent* UCharactersRegistrySubsystem::GetNpcInventory(const FGuid& NpcGUID) const
{
	const FNpcRegistryEntry* Entry = NpcRegistry.Find(NpcGUID);
	return Entry ? Entry->Inventory : nullptr;
}

UNpcContextComponent* UCharactersRegistrySubsystem::GetNpcContextComponent(const FGuid& NpcGUID) const
{
	const FNpcRegistryEntry* Entry = NpcRegistry.Find(NpcGUID);
	return Entry ? Entry->ContextComponent : nullptr;
}


/*=========================================================================
 * LOOKUPS: Reverse Lookup (Character → GUID)
 *=========================================================================*/

FGuid UCharactersRegistrySubsystem::GetNpcGuidByCharacter(ANpcCharacter* Character) const
{
	if (!Character)
		return FGuid();

	const FGuid* GuidPtr = CharacterToGuidMap.Find(Character);
	return GuidPtr ? *GuidPtr : FGuid();
}

FGuid UCharactersRegistrySubsystem::GetNpcGuidByController(ANpcAiController* Controller) const
{
	if (!Controller)
		return FGuid();

	// Linear search (rare operation)
	for (const auto& [Guid, Entry] : NpcRegistry)
	{
		if (Entry.Controller == Controller)
			return Guid;
	}

	return FGuid();
}

FGuid UCharactersRegistrySubsystem::GetNpcGuidByInventory(UInventoryComponent* Inventory) const
{
	if (!Inventory)
		return FGuid();

	const FGuid* GuidPtr = InventoryToGuidMap.Find(Inventory);
	return GuidPtr ? *GuidPtr : FGuid();
}

/*=========================================================================
 * QUERIES: Filter Operations
 *=========================================================================*/

void UCharactersRegistrySubsystem::GetNpcsInRadius(const FVector& CenterLocation, float Radius, TArray<FGuid>& OutGuids) const
{
	OutGuids.Reset();

	const int32 CellRadius = FMath::CeilToInt(Radius / SpatialCellSize) + 1;
	const FIntPoint CenterCell = LocationToCell(CenterLocation);
	const float RadiusSq = Radius * Radius;

	for (int32 dy = -CellRadius; dy <= CellRadius; ++dy)
	{
		for (int32 dx = -CellRadius; dx <= CellRadius; ++dx)
		{
			const TArray<FGuid>* Bucket = SpatialGrid.Find(FIntPoint(CenterCell.X + dx, CenterCell.Y + dy));
			if (!Bucket)
				continue;

			for (const FGuid& Guid : *Bucket)
			{
				const FNpcRegistryEntry* Entry = NpcRegistry.Find(Guid);
				if (!Entry || !Entry->IsAlive())
					continue;

				if (FVector::DistSquared(Entry->LastKnownLocation, CenterLocation) <= RadiusSq)
				{
					OutGuids.Add(Guid);
				}
			}
		}
	}
}

void UCharactersRegistrySubsystem::GetNpcsByClass(TSubclassOf<ANpcCharacter> NpcClass, TArray<FGuid>& OutGuids) const
{
	OutGuids.Reset();

	if (!NpcClass)
		return;

	for (const auto& [Guid, Entry] : NpcRegistry)
	{
		if (Entry.IsAlive() && Entry.Character->IsA(NpcClass))
		{
			OutGuids.Add(Guid);
		}
	}
}

void UCharactersRegistrySubsystem::GetAliveNpcs(TArray<FGuid>& OutGuids) const
{
	OutGuids.Reset();

	for (const auto& [Guid, Entry] : NpcRegistry)
	{
		if (Entry.IsAlive())
		{
			OutGuids.Add(Guid);
		}
	}
}

FGuid UCharactersRegistrySubsystem::FindClosestNpc(const FVector& Location, float MaxDistance) const
{
	TArray<FGuid> Candidates;
	GetNpcsInRadius(Location, MaxDistance, Candidates);

	FGuid ClosestGuid;
	float ClosestDistSq = MaxDistance * MaxDistance;

	for (const FGuid& Guid : Candidates)
	{
		const FNpcRegistryEntry* Entry = NpcRegistry.Find(Guid);
		if (!Entry)
			continue;

		const float DistSq = FVector::DistSquared(Entry->LastKnownLocation, Location);
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestGuid = Guid;
		}
	}

	return ClosestGuid;
}

/*=========================================================================
	 * SPATIAL GRID (uniform grid hash for radius queries)
*=========================================================================*/

FIntPoint UCharactersRegistrySubsystem::LocationToCell(const FVector& Location) const
{
	return FIntPoint(FMath::FloorToInt(Location.X / SpatialCellSize),
	                 FMath::FloorToInt(Location.Y / SpatialCellSize));
}

void UCharactersRegistrySubsystem::AddToSpatialGrid(const FGuid& Guid, const FVector& Location)
{
	const FIntPoint Cell = LocationToCell(Location);
	SpatialGrid.FindOrAdd(Cell).Add(Guid);
	GuidToCellMap.Add(Guid, Cell);
}

void UCharactersRegistrySubsystem::RemoveFromSpatialGrid(const FGuid& Guid)
{
	if (const FIntPoint* Cell = GuidToCellMap.Find(Guid))
	{
		if (TArray<FGuid>* Bucket = SpatialGrid.Find(*Cell))
		{
			Bucket->RemoveSingleSwap(Guid);
			if (Bucket->Num() == 0)
			{
				SpatialGrid.Remove(*Cell);
			}
		}
		GuidToCellMap.Remove(Guid);
	}
}

void UCharactersRegistrySubsystem::UpdateSpatialGridEntry(const FGuid& Guid, const FVector& NewLocation)
{
	const FIntPoint NewCell = LocationToCell(NewLocation);
	const FIntPoint* OldCell = GuidToCellMap.Find(Guid);

	if (OldCell && *OldCell == NewCell)
	{
		return; // same cell, nothing to update
	}

	RemoveFromSpatialGrid(Guid);
	AddToSpatialGrid(Guid, NewLocation);
}

/*=========================================================================
 * BATCH OPERATIONS
 *=========================================================================*/

void UCharactersRegistrySubsystem::BroadcastEventToAllNpcs(const FGameplayTag& EventTag)
{
	TArray<FGuid> Guids;
	NpcRegistry.GetKeys(Guids);

	for (const FGuid& Guid : Guids)
	{
		FNpcRegistryEntry* Entry = NpcRegistry.Find(Guid);
		if (!Entry || !Entry->IsAlive() || !Entry->ContextComponent)
		{
			continue;
		}

		Entry->ContextComponent->SendStateTreeEvent(EventTag);
	}
}

void UCharactersRegistrySubsystem::UpdateAllNpcLocations()
{
	for (auto& [Guid, Entry] : NpcRegistry)
	{
		if (Entry.IsAlive())
		{
			Entry.UpdateLocation();
			UpdateSpatialGridEntry(Guid, Entry.LastKnownLocation);
		}
	}
}

/*=========================================================================
 * UTILITY
 *=========================================================================*/

bool UCharactersRegistrySubsystem::IsNpcAlive(const FGuid& NpcGUID) const
{
	const FNpcRegistryEntry* Entry = NpcRegistry.Find(NpcGUID);
	return Entry && Entry->IsAlive();
}

FString UCharactersRegistrySubsystem::GetRegistryStats() const
{
	int32 TotalNpcs = NpcRegistry.Num();
	int32 AliveNpcs = 0;

	for (const auto& [Guid, Entry] : NpcRegistry)
	{
		if (Entry.IsAlive())
			AliveNpcs++;
	}

	return FString::Printf(TEXT("[NpcRegistry] Total: %d | Alive: %d | Dead: %d"),
	                       TotalNpcs, AliveNpcs, TotalNpcs - AliveNpcs);
}

void UCharactersRegistrySubsystem::ClearRegistry()
{
	NpcRegistry.Reset();
	CharacterToGuidMap.Reset();
	InventoryToGuidMap.Reset();
	ControllerToGuidMap.Reset();
	SpatialGrid.Reset();
	GuidToCellMap.Reset();

	UE_LOG(LogTemp, Warning, TEXT("[NpcRegistry] Registry cleared"));
}
