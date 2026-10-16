#include "Characters/NPCs/AI/NpcAiComponentBase.h"
#include "Characters/NPCs/NpcAiController.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "GameFramework/Pawn.h"

UNpcAiComponentBase::UNpcAiComponentBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNpcAiComponentBase::BeginPlay()
{
	Super::BeginPlay();
	ResolveOwnerReferences();
}

void UNpcAiComponentBase::ResolveOwnerReferences() const
{
	if (!CachedController.IsValid())
	{
		CachedController = Cast<ANpcAiController>(GetOwner());
		if (!CachedController.IsValid())
		{
			if (const APawn* PawnOwner = Cast<APawn>(GetOwner()))
			{
				CachedController = Cast<ANpcAiController>(PawnOwner->GetController());
			}
		}
	}

	if (!CachedPawn.IsValid())
	{
		if (CachedController.IsValid())
		{
			CachedPawn = CachedController->GetPawn();
		}
		else if (APawn* PawnOwner = Cast<APawn>(GetOwner()))
		{
			CachedPawn = PawnOwner;
		}
	}
}

ANpcAiController* UNpcAiComponentBase::GetNpcController() const
{
	if (!CachedController.IsValid())
	{
		ResolveOwnerReferences();
	}
	return CachedController.Get();
}

APawn* UNpcAiComponentBase::GetNpcPawn() const
{
	if (!CachedPawn.IsValid())
	{
		ResolveOwnerReferences();
	}
	return CachedPawn.Get();
}

const UNpcProfile* UNpcAiComponentBase::GetNpcProfile() const
{
	if (const ANpcAiController* Controller = GetNpcController())
	{
		if (const UNpcContextComponent* Context = Controller->GetContextComponent())
		{
			return Context->GetNpcProfile();
		}
	}
	return nullptr;
}
