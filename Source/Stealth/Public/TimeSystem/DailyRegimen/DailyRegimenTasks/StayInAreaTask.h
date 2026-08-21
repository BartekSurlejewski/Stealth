#pragma once

#include "CoreMinimal.h"
#include "DailyRegimenTask.h"
#include "StayInAreaTask.generated.h"

class AActor;
class ATriggerVolume;
/**
 * Task to stay in a specific area for a certain amount of time
 */
UCLASS()
class STEALTH_API UStayInAreaTask : public UDailyRegimenTask
{
	GENERATED_BODY()

public:
	virtual void InitializeTask_Implementation() override;
	virtual void DisposeTask_Implementation() override;
	virtual void PerformByPrisoner_Implementation(UPrisonerNpcContextComponent* PrisonerContext) override;
	UFUNCTION()
	void TargetArea_OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);
	UFUNCTION()
	void TargetArea_OnActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ATriggerVolume> TargetArea;
	UPROPERTY()
	bool bIsPlayerInArea;
};
