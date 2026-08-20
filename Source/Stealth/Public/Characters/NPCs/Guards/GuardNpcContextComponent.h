#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "GuardNpcContextComponent.generated.h"

class APatrolRoute;
class AStealthPlayerState;
class UStateTreeComponent;

UCLASS(ClassGroup = (NPC), meta = (BlueprintSpawnableComponent))
class STEALTH_API UGuardNpcContextComponent : public UNpcContextComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Guard|State")
	int32 GlobalAlarmLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard|Patrol")
	int32 CurrentPatrolIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guard|Patrol")
	bool bMovingForward = true;

public:
	virtual void BeginPlay() override;

	/** Get the currently assigned patrol route */
	UFUNCTION(BlueprintPure, Category = "Guard|Patrol")
	APatrolRoute* GetActivePatrolRoute() const;

	/** Set active patrol route explicitly */
	UFUNCTION(BlueprintCallable, Category = "Guard|Patrol")
	void SetActivePatrolRoute(APatrolRoute* NewRoute);

	/** Find and assign a patrol route from the world matching the location tag */
	UFUNCTION(BlueprintCallable, Category = "Guard|Patrol")
	APatrolRoute* AssignPatrolRouteForLocation(const FGameplayTag& LocationTag);

	/** Get current target waypoint actor along the active route */
	UFUNCTION(BlueprintPure, Category = "Guard|Patrol")
	AActor* GetCurrentPatrolPoint() const;

	/** Advance to the next waypoint along the active route */
	UFUNCTION(BlueprintCallable, Category = "Guard|Patrol")
	void IncrementPatrolIndex();

	/** Resume patrol from current or nearest waypoint when returning from another state */
	UFUNCTION(BlueprintCallable, Category = "Guard|Patrol")
	void ResumePatrol();

	/** Reset patrol progression back to the start */
	UFUNCTION(BlueprintCallable, Category = "Guard|Patrol")
	void ResetPatrol();

	/** Check if guard is currently assigned to a walking patrol */
	UFUNCTION(BlueprintPure, Category = "Guard|Patrol")
	bool IsOnWalkingPatrol() const;

	UFUNCTION(BlueprintCallable, Category = "Guard|Actions")
	void LookAtPlayer();

private:
	UFUNCTION()
	void OnAlarmChanged(int32 NewLevel, const FVector& SourceLocation);

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Guard|Patrol", meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<APatrolRoute> ActivePatrolRoute;
};
