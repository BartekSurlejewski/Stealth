#include "Characters/NPCs/AI/ActivityPointSubsystem.h"
#include "GameFramework/Actor.h"

void UActivityPointSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	ActivityPoints.Empty();
	ActiveClaims.Empty();
	RegisteredActors.Empty();
}

void UActivityPointSubsystem::RegisterActivityPoint(AActor* ActivityActor, const FGameplayTagContainer& ActivityTags, int32 MaxOccupancy)
{
	if (!ActivityActor)
	{
		return;
	}

	FActivityPointEntry& Entry = ActivityPoints.FindOrAdd(ActivityActor);
	Entry.Actor = ActivityActor;
	Entry.Tags = ActivityTags;
	Entry.MaxOccupancy = FMath::Max(1, MaxOccupancy);
	RegisteredActors.AddUnique(ActivityActor);
}

void UActivityPointSubsystem::UnregisterActivityPoint(AActor* ActivityActor)
{
	if (!ActivityActor)
	{
		return;
	}

	if (FActivityPointEntry* Entry = ActivityPoints.Find(ActivityActor))
	{
		for (const FGuid& ClaimerGuid : Entry->CurrentClaimers)
		{
			ActiveClaims.Remove(ClaimerGuid);
		}
	}

	ActivityPoints.Remove(ActivityActor);
	RegisteredActors.Remove(ActivityActor);
}

AActor* UActivityPointSubsystem::ClaimActivityPoint(const FGuid& NpcGuid, const FGameplayTag& ActivityTag, const FVector& RequesterLocation, AActor* PreferredActor)
{
	if (!NpcGuid.IsValid())
	{
		return nullptr;
	}

	// First, check if preferred actor is available and valid
	if (PreferredActor && ActivityPoints.Contains(PreferredActor))
	{
		FActivityPointEntry& Entry = ActivityPoints[PreferredActor];
		if (Entry.Tags.HasTag(ActivityTag) && Entry.CurrentClaimers.Num() < Entry.MaxOccupancy)
		{
			// Release existing claim first if different
			ReleaseActivityPoint(NpcGuid, PreferredActor);

			Entry.CurrentClaimers.AddUnique(NpcGuid);
			FActivitySlotClaim& Claim = ActiveClaims.FindOrAdd(NpcGuid);
			Claim.ClaimerGuid = NpcGuid;
			Claim.TargetActor = PreferredActor;
			Claim.ActivityTag = ActivityTag;
			Claim.ClaimTimestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
			return PreferredActor;
		}
	}

	// Search for best matching free activity point (closest to requester)
	AActor* BestActor = nullptr;
	float BestDistSq = MAX_FLT;

	for (auto It = ActivityPoints.CreateIterator(); It; ++It)
	{
		FActivityPointEntry& Entry = It.Value();
		AActor* Candidate = Entry.Actor.Get();
		if (!Candidate)
		{
			It.RemoveCurrent();
			continue;
		}

		if (Entry.Tags.HasTag(ActivityTag) && Entry.CurrentClaimers.Num() < Entry.MaxOccupancy)
		{
			float DistSq = RequesterLocation.IsZero() ? 0.0f : FVector::DistSquared(Candidate->GetActorLocation(), RequesterLocation);
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestActor = Candidate;
			}
		}
	}

	if (BestActor)
	{
		ReleaseActivityPoint(NpcGuid, BestActor);

		FActivityPointEntry& Entry = ActivityPoints[BestActor];
		Entry.CurrentClaimers.AddUnique(NpcGuid);

		FActivitySlotClaim& Claim = ActiveClaims.FindOrAdd(NpcGuid);
		Claim.ClaimerGuid = NpcGuid;
		Claim.TargetActor = BestActor;
		Claim.ActivityTag = ActivityTag;
		Claim.ClaimTimestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		return BestActor;
	}

	return nullptr;
}

void UActivityPointSubsystem::ReleaseActivityPoint(const FGuid& NpcGuid, AActor* SpecificActor)
{
	if (!NpcGuid.IsValid())
	{
		return;
	}

	if (FActivitySlotClaim* ExistingClaim = ActiveClaims.Find(NpcGuid))
	{
		AActor* ClaimedActor = ExistingClaim->TargetActor.Get();
		if (ClaimedActor && (!SpecificActor || SpecificActor == ClaimedActor))
		{
			if (FActivityPointEntry* Entry = ActivityPoints.Find(ClaimedActor))
			{
				Entry->CurrentClaimers.Remove(NpcGuid);
			}
		}
		ActiveClaims.Remove(NpcGuid);
	}
}

bool UActivityPointSubsystem::IsActivityPointOccupied(const AActor* ActivityActor) const
{
	if (!ActivityActor)
	{
		return false;
	}

	if (const FActivityPointEntry* Entry = ActivityPoints.Find(ActivityActor))
	{
		return Entry->CurrentClaimers.Num() >= Entry->MaxOccupancy;
	}
	return false;
}

bool UActivityPointSubsystem::GetClaimForNpc(const FGuid& NpcGuid, FActivitySlotClaim& OutClaim) const
{
	if (const FActivitySlotClaim* Found = ActiveClaims.Find(NpcGuid))
	{
		OutClaim = *Found;
		return true;
	}
	return false;
}
