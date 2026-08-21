#include "Characters/Player/Components/StealthCharacterCollisionsComponent.h"

#include "Characters/Player/StealthPlayerCharacter.h"
#include "Characters/Player/StealthPlayerState.h"

UStealthCharacterCollisionsComponent::UStealthCharacterCollisionsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStealthCharacterCollisionsComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AStealthPlayerCharacter* PlayerCharacter = Cast<AStealthPlayerCharacter>(GetOwner()))
	{
		PlayerState = Cast<AStealthPlayerState>(PlayerCharacter->GetPlayerState());
	}
}

void UStealthCharacterCollisionsComponent::OnBeginOverlap(AActor* OtherActor)
{
	if (OtherActor && OtherActor != GetOwner())
	{
		//TODO: define tags in a single place to avoid magic strings
		if (OtherActor->ActorHasTag("IllegalArea") && PlayerState)
		{
			IllegalAreaCollidersOverlapCount++;
			if (IllegalAreaCollidersOverlapCount == 1)
			{
				PlayerState->SetIsInRestrictedArea(true);
				UE_LOG(LogTemp, Warning, TEXT("Player entered illegal area!"));
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Player Overlapped by %s"), *OtherActor->GetName());
	}
}

void UStealthCharacterCollisionsComponent::OnEndOverlap(AActor* OtherActor)
{
	if (OtherActor && OtherActor != GetOwner())
	{
		if (OtherActor->ActorHasTag("IllegalArea") && PlayerState)
		{
			IllegalAreaCollidersOverlapCount--;
			if (IllegalAreaCollidersOverlapCount == 0)
			{
				PlayerState->SetIsInRestrictedArea(false);
				UE_LOG(LogTemp, Warning, TEXT("Player left illegal area!"));
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Player End Overlap with %s"), *OtherActor->GetName());
	}
}
