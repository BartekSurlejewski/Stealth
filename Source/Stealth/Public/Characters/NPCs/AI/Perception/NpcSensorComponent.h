#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/AI/NpcAiComponentBase.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "Perception/AIPerceptionTypes.h"
#include "NpcSensorComponent.generated.h"

/**
 * Sensor component converting Unreal AIPerception stimuli into per-actor FNpcTrackedTarget
 * records in FNpcSensorData and raising FNpcSensorEvent notifications.
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class STEALTH_API UNpcSensorComponent : public UNpcAiComponentBase
{
	GENERATED_BODY()

public:
	UNpcSensorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Primary stimulus entry point called by ANpcAiController */
	void HandleStimulus(AActor* Actor, const FAIStimulus& Stimulus);

	/** Called by ANpcAiController when a tracked actor is forgotten by AIPerception */
	void HandleForgotten(AActor* Actor);

	/** Access raw tracked sensor data */
	const FNpcSensorData& GetData() const { return SensorData; }

	/** Multicast delegate for sensor events */
	FOnNpcSensorEvent OnSensorEvent;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI|Sensor")
	FNpcSensorData SensorData;
};
