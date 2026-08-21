#include "Housing/HousingSubsystem.h"
#include "Characters/NPCs/CharactersRegistrySubsystem.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Housing/HouseComponent.h"
#include "Stealth/Stealth.h"

void UHousingSubsystem::RegisterHouse(UHouseComponent* HouseComponent)
{
	if (!HouseComponent)
	{
		return;
	}

	const FName HouseId = HouseComponent->GetHouseId();
	if (HouseId.IsNone())
	{
		UE_LOG(LogStealth, Warning, TEXT("RegisterHouse: HouseComponent on '%s' has an empty HouseId!"), *HouseComponent->GetOwner()->GetName());
	}

	if (HouseIdToIndexMap.Contains(HouseId))
	{
		UE_LOG(LogStealth, Error, TEXT("House with id '%s' is already registered!"), *HouseId.ToString());
		return;
	}

	const int32 HouseIndex = Houses.AddUnique(HouseComponent);
	HouseIdToIndexMap.Add(HouseId, HouseIndex);

	if (HouseComponent->IsPlayerOwned())
	{
		PlayerHouseIndex = HouseIndex;
	}

	for (const FGuid& OwnerGuid : HouseComponent->GetOwnerGuids())
	{
		if (OwnerGuid.IsValid())
		{
			NpcOwnerToHouseIndexMap.Add(OwnerGuid, HouseIndex);
		}
	}
}

void UHousingSubsystem::UnregisterHouse(UHouseComponent* HouseComponent)
{
	if (!HouseComponent) return;

	Houses.Remove(HouseComponent);
	HouseIdToIndexMap.Remove(HouseComponent->GetHouseId());

	for (const FGuid& OwnerGuid : HouseComponent->GetOwnerGuids())
	{
		NpcOwnerToHouseIndexMap.Remove(OwnerGuid);
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
	NpcOwnerToHouseIndexMap.Empty();
	for (int32 i = 0; i < Houses.Num(); ++i)
	{
		if (const UHouseComponent* House = Houses[i])
		{
			for (const FGuid& OwnerGuid : House->GetOwnerGuids())
			{
				if (OwnerGuid.IsValid())
				{
					NpcOwnerToHouseIndexMap.Add(OwnerGuid, i);
				}
			}
		}
	}
}

UHouseComponent* UHousingSubsystem::GetHouseById(const FName& HouseId) const
{
	const int32 FoundIndex = HouseIdToIndexMap.FindRef(HouseId);
	if (FoundIndex >= 0 && FoundIndex < Houses.Num())
	{
		return Houses[FoundIndex];
	}
	return nullptr;
}

UHouseComponent* UHousingSubsystem::GetHouseForNpc(const FGuid& NpcGuid) const
{
	const int32 FoundIndex = NpcOwnerToHouseIndexMap.FindRef(NpcGuid);

	if (FoundIndex >= 0 && FoundIndex < Houses.Num())
	{
		return Houses[FoundIndex];
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

bool UHousingSubsystem::IsActorAllowedInHouse(const FName& HouseId, const AActor* Actor) const
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

void UHousingSubsystem::ReportCrime(const FHouseCrimeReport& CrimeReport) const
{
	OnCrimeReported.Broadcast(CrimeReport);

	UCharactersRegistrySubsystem* Registry = UCharactersRegistrySubsystem::Get(this);
	if (!Registry) return;

	UHouseComponent* House = GetHouseById(CrimeReport.HouseId);
	if (!House) return;

	// Build structured AI Crime Payload
	FAiCrimeEventPayload BasePayload;
	BasePayload.HouseId = CrimeReport.HouseId;
	BasePayload.CrimeType = CrimeReport.CrimeType;
	BasePayload.CrimeLocation = CrimeReport.CrimeLocation;
	BasePayload.Perpetrator = CrimeReport.Perpetrator;
	BasePayload.VictimOrTarget = CrimeReport.VictimOrTarget;
	BasePayload.bIsPrimaryInvestigator = false;

	// 1. Alert Owners if they are alive and have context
	for (const FGuid& OwnerGuid : House->GetOwnerGuids())
	{
		if (UNpcContextComponent* Context = Registry->GetNpcContextComponent(OwnerGuid))
		{
			Context->HandleCrimeReported(BasePayload);

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
	Registry->GetNpcsInRadius(CrimeReport.CrimeLocation, 2000.0f, NearbyGuids);

	// Find closest responder among nearby NPCs
	FGuid ClosestResponderGuid;
	float ClosestDistSq = MAX_FLT;

	for (const FGuid& NearbyGuid : NearbyGuids)
	{
		if (const ANpcCharacter* NearbyNpc = Registry->GetNpcCharacter(NearbyGuid))
		{
			const float DistSq = FVector::DistSquared(NearbyNpc->GetActorLocation(), CrimeReport.CrimeLocation);
			if (DistSq < ClosestDistSq)
			{
				ClosestDistSq = DistSq;
				ClosestResponderGuid = NearbyGuid;
			}
		}
	}

	for (const FGuid& NearbyGuid : NearbyGuids)
	{
		if (UNpcContextComponent* Context = Registry->GetNpcContextComponent(NearbyGuid))
		{
			FAiCrimeEventPayload ResponderPayload = BasePayload;
			ResponderPayload.bIsPrimaryInvestigator = (NearbyGuid == ClosestResponderGuid);

			Context->HandleCrimeReported(ResponderPayload);

			if (IntrusionStateTreeEventTag.IsValid())
			{
				Context->SendStateTreeEvent(IntrusionStateTreeEventTag);
			}
		}
	}
}

void UHousingSubsystem::ReportIntrusion(const FName& HouseId, AActor* Intruder) const
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
