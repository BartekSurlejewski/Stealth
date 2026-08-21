#include "ChemistryEffect.h"
#include "ChemistryReceiverInterface.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UWorld* UChemistryEffect::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}

	return GetOuter() ? GetOuter()->GetWorld() : nullptr;
}

void UChemistryEffect::ExecuteEffect_Implementation(const FGameplayTag& EffectTag, const FElementApplication& Context, AActor* AffectedActor)
{
	// Default base behavior: ensure interface call is routed to affected actor if not handled prior
	if (IsValid(AffectedActor) && AffectedActor->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
	{
		IChemistryReceiverInterface::Execute_OnReceiveChemistryEffect(AffectedActor, EffectTag, Context);
	}
}
