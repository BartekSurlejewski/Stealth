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

	FTimerHandle PatrolTimer;
	int32 CurrentTargetIndex = 0;

public:
	UNpcPatrolComponent();

public:
	UFUNCTION(BlueprintCallable)
	AActor* GetTarget();
};
