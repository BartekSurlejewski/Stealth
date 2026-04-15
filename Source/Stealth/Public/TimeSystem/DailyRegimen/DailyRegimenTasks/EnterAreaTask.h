#pragma once

#include "CoreMinimal.h"
#include "DailyRegimenTask.h"
#include "EnterAreaTask.generated.h"

class ATriggerVolume;

UCLASS(Blueprintable)
class STEALTH_API UEnterAreaTask : public UDailyRegimenTask
{
	GENERATED_BODY()

public:
	virtual void InitializeTask_Implementation() override;
	virtual void DisposeTask_Implementation() override;
	UFUNCTION()
	void TargetArea_OnActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ATriggerVolume> TargetArea;
};
