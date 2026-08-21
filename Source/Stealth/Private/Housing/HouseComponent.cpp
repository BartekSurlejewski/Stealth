#include "Housing/HouseComponent.h"

#include "Characters/NPCs/NpcCharacter.h"
#include "Characters/Player/StealthPlayerCharacter.h"
#include "Housing/HousingSubsystem.h"

UHouseComponent::UHouseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UHouseComponent::BeginPlay()
{
	Super::BeginPlay();

	// Convert any direct actor references to GUIDs
	for (const TObjectPtr<ANpcCharacter>& NpcOwner : InitialNpcOwners)
	{
		if (NpcOwner && NpcOwner->GetNpcGUID().IsValid())
		{
			OwnerGuids.AddUnique(NpcOwner->GetNpcGUID());
		}
	}

	// Bind overlap events if trigger component exists
	if (HouseVolumeTrigger)
	{
		HouseVolumeTrigger->OnComponentBeginOverlap.AddDynamic(this, &UHouseComponent::HandleVolumeOverlapBegin);
		HouseVolumeTrigger->OnComponentEndOverlap.AddDynamic(this, &UHouseComponent::HandleVolumeOverlapEnd);
	}

	if (UHousingSubsystem* HousingSubsystem = UHousingSubsystem::Get(this))
	{
		HousingSubsystem->RegisterHouse(this);
	}
}

void UHouseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UHousingSubsystem* HousingSubsystem = UHousingSubsystem::Get(this))
	{
		HousingSubsystem->UnregisterHouse(this);
	}

	if (HouseVolumeTrigger)
	{
		HouseVolumeTrigger->OnComponentBeginOverlap.RemoveAll(this);
		HouseVolumeTrigger->OnComponentEndOverlap.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UHouseComponent::SetHouseVolumeTrigger(UPrimitiveComponent* InVolumeTrigger)
{
	if (HouseVolumeTrigger)
	{
		HouseVolumeTrigger->OnComponentBeginOverlap.RemoveAll(this);
		HouseVolumeTrigger->OnComponentEndOverlap.RemoveAll(this);
	}

	HouseVolumeTrigger = InVolumeTrigger;

	if (HouseVolumeTrigger)
	{
		HouseVolumeTrigger->OnComponentBeginOverlap.AddDynamic(this, &UHouseComponent::HandleVolumeOverlapBegin);
		HouseVolumeTrigger->OnComponentEndOverlap.AddDynamic(this, &UHouseComponent::HandleVolumeOverlapEnd);
	}
}

EHouseAccessLevel UHouseComponent::GetActorAccessLevel(const AActor* Actor) const
{
	if (!Actor) return EHouseAccessLevel::Forbidden;

	// Check Player
	if (Actor->IsA<AStealthPlayerCharacter>())
	{
		return bPlayerOwned ? EHouseAccessLevel::Owner : (bPublicDuringDay ? EHouseAccessLevel::VisitorAllowed : EHouseAccessLevel::Forbidden);
	}

	// Check NPC
	if (const ANpcCharacter* Npc = Cast<ANpcCharacter>(Actor))
	{
		if (OwnerGuids.Contains(Npc->GetNpcGUID()))
		{
			return EHouseAccessLevel::Owner;
		}
	}

	return bPublicDuringDay ? EHouseAccessLevel::VisitorAllowed : EHouseAccessLevel::Forbidden;
}

bool UHouseComponent::IsActorAllowed(const AActor* Actor) const
{
	const EHouseAccessLevel AccessLevel = GetActorAccessLevel(Actor);
	return AccessLevel != EHouseAccessLevel::Forbidden;
}

bool UHouseComponent::IsOwner(const AActor* Actor) const
{
	return GetActorAccessLevel(Actor) == EHouseAccessLevel::Owner;
}

AActor* UHouseComponent::GetBedForNpc(const FGuid& NpcGuid) const
{
	if (const TObjectPtr<AActor>* FoundBed = ActivityPoints.AssignedBeds.Find(NpcGuid))
	{
		return FoundBed->Get();
	}
	return nullptr;
}

void UHouseComponent::AssignBedToNpc(const FGuid& NpcGuid, AActor* BedActor)
{
	if (NpcGuid.IsValid())
	{
		ActivityPoints.AssignedBeds.Add(NpcGuid, BedActor);
	}
}

void UHouseComponent::NotifyCrime(const EHouseCrimeType& CrimeType, AActor* Perpetrator, AActor* TargetObject)
{
	FHouseCrimeReport Report;
	Report.HouseId = HouseId;
	Report.CrimeType = CrimeType;
	Report.Perpetrator = Perpetrator;
	Report.VictimOrTarget = TargetObject;
	Report.CrimeLocation = TargetObject ? TargetObject->GetActorLocation() : (Perpetrator ? Perpetrator->GetActorLocation() : GetOwner()->GetActorLocation());
	Report.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	OnCrimeDetected.Broadcast(this, Report);

	if (UHousingSubsystem* HousingSubsystem = UHousingSubsystem::Get(this))
	{
		HousingSubsystem->ReportCrime(Report);
	}
}

void UHouseComponent::HandleVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                               bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || CurrentOccupants.Contains(OtherActor))
	{
		return;
	}

	CurrentOccupants.Add(OtherActor);
	OnActorEntered.Broadcast(this, OtherActor);

	if (UHousingSubsystem* HousingSubsystem = UHousingSubsystem::Get(this))
	{
		HousingSubsystem->OnActorEnteredHouse(this, OtherActor);
	}

	// Check if this is an unauthorized intrusion
	if (!IsActorAllowed(OtherActor))
	{
		NotifyCrime(EHouseCrimeType::Trespassing, OtherActor, GetOwner());
	}
}

void UHouseComponent::HandleVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor || !CurrentOccupants.Contains(OtherActor))
	{
		return;
	}

	CurrentOccupants.Remove(OtherActor);
	OnActorExited.Broadcast(this, OtherActor);

	if (UHousingSubsystem* HousingSubsystem = UHousingSubsystem::Get(this))
	{
		HousingSubsystem->OnActorLeftHouse(this, OtherActor);
	}
}
