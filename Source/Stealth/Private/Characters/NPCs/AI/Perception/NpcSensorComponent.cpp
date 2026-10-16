#include "Characters/NPCs/AI/Perception/NpcSensorComponent.h"
#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Touch.h"
#include "Perception/AISense_Damage.h"
#include "Perception/AISense_Prediction.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UNpcSensorComponent::UNpcSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void UNpcSensorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SensorData.PruneInvalid();

	const APawn* Pawn = GetNpcPawn();
	const FVector MyLoc = Pawn ? Pawn->GetActorLocation() : FVector::ZeroVector;

	for (FNpcTrackedTarget& Target : SensorData.Targets)
	{
		if (const AActor* TargetActor = Target.Actor.Get())
		{
			Target.Distance = FVector::Dist(MyLoc, TargetActor->GetActorLocation());
		}
		else if (Target.bHasLocation)
		{
			Target.Distance = FVector::Dist(MyLoc, Target.LastKnownLocation);
		}
	}
}

void UNpcSensorComponent::HandleStimulus(AActor* Actor, const FAIStimulus& Stimulus)
{
	if (!Actor)
	{
		return;
	}

	const FAISenseID SightSenseID = UAISense::GetSenseID<UAISense_Sight>();
	const FAISenseID HearingSenseID = UAISense::GetSenseID<UAISense_Hearing>();
	const FAISenseID TouchSenseID = UAISense::GetSenseID<UAISense_Touch>();
	const FAISenseID DamageSenseID = UAISense::GetSenseID<UAISense_Damage>();
	const FAISenseID PredictionSenseID = UAISense::GetSenseID<UAISense_Prediction>();

	const double Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;
	const FGameplayTag StimulusTag = Stimulus.Tag.IsNone() ? FGameplayTag::EmptyTag : FGameplayTag::RequestGameplayTag(Stimulus.Tag, false);

	if (Stimulus.Type == SightSenseID)
	{
		FNpcTrackedTarget& Target = SensorData.FindOrAdd(Actor);
		Target.bHasLocation = true;
		Target.LastKnownLocation = Stimulus.StimulusLocation;
		Target.LastStimulusTag = StimulusTag;
		Target.LastStimulusStrength = Stimulus.Strength;
		Target.LastSensedTime = Now;

		FNpcSensorEvent Event;
		Event.Actor = Actor;
		Event.Location = Target.LastKnownLocation;
		Event.Strength = Stimulus.Strength;
		Event.Tag = StimulusTag;

		if (Stimulus.WasSuccessfullySensed())
		{
			Target.CurrentSenses |= ENpcSenseFlags::Sight;
			Target.LastSeenTime = Now;
			Event.Type = ENpcSensorEventType::SightGained;
		}
		else
		{
			Target.CurrentSenses &= ~ENpcSenseFlags::Sight;
			Event.Type = ENpcSensorEventType::SightLost;
		}

		OnSensorEvent.Broadcast(Event);
	}
	else if (Stimulus.Type == HearingSenseID)
	{
		FNpcTrackedTarget& Target = SensorData.FindOrAdd(Actor);
		Target.bHasLocation = true;
		Target.LastKnownLocation = Stimulus.StimulusLocation;
		Target.CurrentSenses |= ENpcSenseFlags::Hearing;
		Target.LastHeardTime = Now;
		Target.LastSensedTime = Now;
		Target.LastStimulusTag = StimulusTag;
		Target.LastStimulusStrength = Stimulus.Strength;

		const UNpcProfile* Profile = GetNpcProfile();
		const ENpcNoiseType NoiseType = Profile ? Profile->ClassifyNoise(Stimulus.Tag, Stimulus.Strength) : ENpcNoiseType::Subtle;

		FNpcSensorEvent Event;
		Event.Type = ENpcSensorEventType::Heard;
		Event.Actor = Actor;
		Event.Location = Target.LastKnownLocation;
		Event.Strength = Stimulus.Strength;
		Event.Tag = StimulusTag;
		Event.NoiseType = NoiseType;

		OnSensorEvent.Broadcast(Event);
	}
	else if (Stimulus.Type == TouchSenseID)
	{
		FNpcTrackedTarget& Target = SensorData.FindOrAdd(Actor);
		Target.bHasLocation = true;
		Target.LastKnownLocation = Stimulus.StimulusLocation;
		Target.CurrentSenses |= ENpcSenseFlags::Touch;
		Target.LastSensedTime = Now;
		Target.LastStimulusTag = StimulusTag;
		Target.LastStimulusStrength = Stimulus.Strength;

		FNpcSensorEvent Event;
		Event.Type = ENpcSensorEventType::Touched;
		Event.Actor = Actor;
		Event.Location = Target.LastKnownLocation;
		Event.Strength = Stimulus.Strength;
		Event.Tag = StimulusTag;

		OnSensorEvent.Broadcast(Event);
	}
	else if (Stimulus.Type == DamageSenseID)
	{
		FNpcTrackedTarget& Target = SensorData.FindOrAdd(Actor);
		Target.bHasLocation = true;
		Target.LastKnownLocation = Stimulus.StimulusLocation;
		Target.CurrentSenses |= ENpcSenseFlags::Damage;
		Target.LastSensedTime = Now;
		Target.LastStimulusTag = StimulusTag;
		Target.LastStimulusStrength = Stimulus.Strength;

		FNpcSensorEvent Event;
		Event.Type = ENpcSensorEventType::Damaged;
		Event.Actor = Actor;
		Event.Location = Target.LastKnownLocation;
		Event.Strength = Stimulus.Strength;
		Event.Tag = StimulusTag;

		OnSensorEvent.Broadcast(Event);
	}
	else if (Stimulus.Type == PredictionSenseID)
	{
		FNpcTrackedTarget& Target = SensorData.FindOrAdd(Actor);
		Target.bHasLocation = true;
		Target.LastKnownLocation = Stimulus.StimulusLocation;
		Target.CurrentSenses |= ENpcSenseFlags::Prediction;
		Target.LastSensedTime = Now;
		Target.LastStimulusTag = StimulusTag;
		Target.LastStimulusStrength = Stimulus.Strength;
	}
}

void UNpcSensorComponent::HandleForgotten(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	if (SensorData.Find(Actor))
	{
		SensorData.Remove(Actor);

		FNpcSensorEvent Event;
		Event.Type = ENpcSensorEventType::Forgotten;
		Event.Actor = Actor;
		OnSensorEvent.Broadcast(Event);
	}
}
