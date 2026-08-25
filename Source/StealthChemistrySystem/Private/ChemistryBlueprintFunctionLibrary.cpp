#include "ChemistryBlueprintFunctionLibrary.h"
#include "ChemistrySubsystem.h"
#include "ChemistryReceiverInterface.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

void UChemistryBlueprintFunctionLibrary::ApplyElement(const UObject* WorldContextObject, const FElementApplication& Application)
{
	if (!WorldContextObject)
	{
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return;
	}

	if (UChemistrySubsystem* Subsystem = World->GetSubsystem<UChemistrySubsystem>())
	{
		Subsystem->ApplyElement(Application);
	}
}

void UChemistryBlueprintFunctionLibrary::ApplyElementToActor(const UObject* WorldContextObject, FGameplayTag ElementTag, AActor* TargetActor, AActor* Instigator, float Magnitude)
{
	FElementApplication App;
	App.ElementTag = ElementTag;
	App.TargetActor = TargetActor;
	App.Instigator = Instigator;
	App.Magnitude = Magnitude;

	if (IsValid(TargetActor))
	{
		App.Location = TargetActor->GetActorLocation();
	}

	ApplyElement(WorldContextObject, App);
}

void UChemistryBlueprintFunctionLibrary::ApplyElementInRadius(const UObject* WorldContextObject, FGameplayTag ElementTag, FVector Location, float Radius, AActor* Instigator,
                                                              float Magnitude)
{
	FElementApplication App;
	App.ElementTag = ElementTag;
	App.Location = Location;
	App.Radius = Radius;
	App.Instigator = Instigator;
	App.Magnitude = Magnitude;

	ApplyElement(WorldContextObject, App);
}

FGameplayTagContainer UChemistryBlueprintFunctionLibrary::GetActorMaterialTags(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return FGameplayTagContainer();
	}

	if (const UWorld* World = Actor->GetWorld())
	{
		if (const UChemistrySubsystem* Subsystem = World->GetSubsystem<UChemistrySubsystem>())
		{
			return Subsystem->GetMaterialTagsForActor(Actor);
		}
	}

	// Fallback if subsystem is not yet initialized
	FGameplayTagContainer CombinedTags;
	if (Actor->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
	{
		CombinedTags.AppendTags(IChemistryReceiverInterface::Execute_GetMaterialTags(Actor));
	}

	for (UActorComponent* Component : Actor->GetComponents())
	{
		if (IsValid(Component) && Component->GetClass()->ImplementsInterface(UChemistryReceiverInterface::StaticClass()))
		{
			CombinedTags.AppendTags(IChemistryReceiverInterface::Execute_GetMaterialTags(Component));
		}
	}

	return CombinedTags;
}

void UChemistryBlueprintFunctionLibrary::ApplyElementFromHitResult(const UObject* WorldContextObject, FGameplayTag ElementTag, const FHitResult& Hit, AActor* Instigator,
                                                                   float Magnitude)
{
	FElementApplication App;
	App.ElementTag = ElementTag;
	App.Instigator = Instigator;
	App.Magnitude = Magnitude;
	App.TargetActor = Hit.GetActor();
	App.HitComponent = Hit.GetComponent();
	// ImpactPoint is only meaningful for a genuine blocking hit; fall back to the sweep's
	// query location otherwise (e.g. overlap-only queries).
	App.Location = Hit.bBlockingHit ? Hit.ImpactPoint : Hit.Location;

	ApplyElement(WorldContextObject, App);
}
