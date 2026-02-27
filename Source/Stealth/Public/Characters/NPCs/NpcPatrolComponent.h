#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NpcPatrolComponent.generated.h"

struct FAIRequestID;

namespace EPathFollowingResult
{
	enum Type : int;
}

class ANpcAiController;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UNpcPatrolComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Patrolling")
	TArray<TObjectPtr<AActor>> PatrolTargets;
	UPROPERTY()
	TObjectPtr<ANpcAiController> NpcController;

	int32 CurrentTargetIndex = 0;

public:
	UNpcPatrolComponent();

private:
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
UFUNCTION()
	void MoveToCurrentTarget();
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
