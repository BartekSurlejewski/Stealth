#include "Characters/Player/Components/StealthCharacterCollisionsComponent.h"

#include "Characters/Player/StealthCharacter.h"
#include "Characters/Player/StealthPlayerState.h"

UStealthCharacterCollisionsComponent::UStealthCharacterCollisionsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStealthCharacterCollisionsComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AStealthCharacter* PlayerCharacter = Cast<AStealthCharacter>(GetOwner()))
	{
		PlayerState = Cast<AStealthPlayerState>(PlayerCharacter->GetPlayerState());
	}
}

void UStealthCharacterCollisionsComponent::OnBeginOverlap(AActor* OtherActor)
{
	if (OtherActor && OtherActor != GetOwner())
	{
		if (OtherActor->ActorHasTag("IllegalArea") && PlayerState)
		{
			PlayerState->SetIsInRestrictedArea(true);
			UE_LOG(LogTemp, Warning, TEXT("Player entered illegal area!"));
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
			PlayerState->SetIsInRestrictedArea(false);
			UE_LOG(LogTemp, Warning, TEXT("Player left illegal area!"));
		}
		UE_LOG(LogTemp, Warning, TEXT("Player End Overlap with %s"), *OtherActor->GetName());
	}
}
