#include "Housing/HousingSubsystem.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Housing/HouseComponent.h"
#include "Stealth/Stealth.h"

void UHousingSubsystem::RegisterHouse(UHouseComponent* HouseComponent)
{
	if (!HouseComponent) return;

	const FName HouseId = HouseComponent->GetHouseId();
	if (HouseId.IsNone())
	{
		UE_LOG(LogStealth, Warning, TEXT("RegisterHouse: HouseComponent on '%s' has an empty HouseId!"), *HouseComponent->GetOwner()->GetName());
	}

	if (HouseIdMap.Contains(HouseId))
	{
		UE_LOG(LogStealth, Error, TEXT("House with id '%s' is already registered!"), *HouseId.ToString());
		return;
	}

	Houses.AddUnique(HouseComponent);
	HouseIdMap.Add(HouseId, HouseComponent);

	for (const FGuid& OwnerGuid : HouseComponent->GetOwnerGuids())
	{
		if (OwnerGuid.IsValid())
		{
			NpcOwnerToHouseMap.Add(OwnerGuid, HouseComponent);
		}
	}
}

void UHousingSubsystem::UnregisterHouse(UHouseComponent* HouseComponent)
{
	if (!HouseComponent) return;

	Houses.Remove(HouseComponent);
	HouseIdMap.Remove(HouseComponent->GetHouseId());

	for (const FGuid& OwnerGuid : HouseComponent->GetOwnerGuids())
	{
		NpcOwnerToHouseMap.Remove(OwnerGuid);
	}

	// Remove any occupancy references
	for (auto It = ActorOccupancyMap.CreateIterator(); It; ++It)
	{
		if (It.Value() == HouseComponent)
		{
			It.RemoveCurrent();
		}
	}
}

void UHousingSubsystem::RebuildOwnerIndexMap()
{
	NpcOwnerToHouseMap.Empty();
	for (UHouseComponent* House : Houses)
	{
		if (House)
		{
			for (const FGuid& OwnerGuid : House->GetOwnerGuids())
			{
				if (OwnerGuid.IsValid())
				{
					NpcOwnerToHouseMap.Add(OwnerGuid, House);
				}
			}
		}
	}
}

UHouseComponent* UHousingSubsystem::GetHouseById(FName HouseId) const
{
	if (const TObjectPtr<UHouseComponent>* Found = HouseIdMap.Find(HouseId))
	{
		return Found->Get();
	}
	return nullptr;
}

UHouseComponent* UHousingSubsystem::GetHouseForNpc(const FGuid& NpcGuid) const
{
	if (const TObjectPtr<UHouseComponent>* Found = NpcOwnerToHouseMap.Find(NpcGuid))
	{
		return Found->Get();
	}
	return nullptr;
}

UHouseComponent* UHousingSubsystem::GetCurrentHouseForActor(const AActor* Actor) const
{
	if (!Actor) return nullptr;

	if (const TObjectPtr<UHouseComponent>* Found = ActorOccupancyMap.Find(Actor))
	{
		return Found->Get();
	}
	return nullptr;
}

bool UHousingSubsystem::IsActorAllowedInHouse(FName HouseId, AActor* Actor) const
{
	if (const UHouseComponent* House = GetHouseById(HouseId))
	{
		return House->IsActorAllowed(Actor);
	}
	return true;
}

bool UHousingSubsystem::IsActorTrespassing(const AActor* Actor) const
{
	if (const UHouseComponent* CurrentHouse = GetCurrentHouseForActor(Actor))
	{
		return !CurrentHouse->IsActorAllowed(Actor);
	}
	return false;
}

AActor* UHousingSubsystem::GetNpcAssignedBed(const FGuid& NpcGuid) const
{
	if (const UHouseComponent* House = GetHouseForNpc(NpcGuid))
	{
		return House->GetBedForNpc(NpcGuid);
	}
	return nullptr;
}

FVector UHousingSubsystem::GetNpcHomeLocation(const FGuid& NpcGuid) const
{
	if (const UHouseComponent* House = GetHouseForNpc(NpcGuid))
	{
		if (const AActor* Bed = House->GetBedForNpc(NpcGuid))
		{
			return Bed->GetActorLocation();
		}
		return House->GetOwner()->GetActorLocation();
	}
	return FVector::ZeroVector;
}

void UHousingSubsystem::ReportCrime(const FHouseCrimeReport& CrimeReport)
{
	OnCrimeReported.Broadcast(CrimeReport);

	UCharactersRegistrySubsystem* Registry = UCharactersRegistrySubsystem::Get(this);
	if (!Registry) return;

	UHouseComponent* House = GetHouseById(CrimeReport.HouseId);
	if (!House) return;

	// 1. Alert Owners if they are alive and have context
	for (const FGuid& OwnerGuid : House->GetOwnerGuids())
	{
		if (UNpcContextComponent* Context = Registry->GetNpcContextComponent(OwnerGuid))
		{
			if (CrimeReport.CrimeType == EHouseCrimeType::Theft && TheftStateTreeEventTag.IsValid())
			{
				Context->SendStateTreeEvent(TheftStateTreeEventTag);
			}
			else if (IntrusionStateTreeEventTag.IsValid())
			{
				Context->SendStateTreeEvent(IntrusionStateTreeEventTag);
			}
		}
	}

	// 2. Alert nearby NPCs / Guards within hearing/awareness radius of the crime location
	TArray<FGuid> NearbyGuids;
	Registry->GetNpcsInRadius(CrimeReport.CrimeLocation, 1500.0f, NearbyGuids);
	for (const FGuid& NearbyGuid : NearbyGuids)
	{
		if (UNpcContextComponent* Context = Registry->GetNpcContextComponent(NearbyGuid))
		{
			// If nearby NPCs see or hear the crime, trigger suspicion/alert
			if (IntrusionStateTreeEventTag.IsValid())
			{
				Context->SendStateTreeEvent(IntrusionStateTreeEventTag);
			}
		}
	}
}

void UHousingSubsystem::ReportIntrusion(FName HouseId, AActor* Intruder)
{
	if (UHouseComponent* House = GetHouseById(HouseId))
	{
		House->NotifyCrime(EHouseCrimeType::Trespassing, Intruder, House->GetOwner());
	}
}

void UHousingSubsystem::OnActorEnteredHouse(UHouseComponent* House, AActor* Actor)
{
	if (Actor && House)
	{
		ActorOccupancyMap.Add(Actor, House);
	}
}

void UHousingSubsystem::OnActorLeftHouse(UHouseComponent* House, AActor* Actor)
{
	if (Actor)
	{
		ActorOccupancyMap.Remove(Actor);
	}
}
