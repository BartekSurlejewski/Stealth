#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Components/ActorComponent.h"
#include "GuardNpcContextComponent.generated.h"

class AStealthPlayerState;
class UStateTreeComponent;
class UNpcPatrolComponent;
struct FAIStimulus;
class UNpcProfile;

UCLASS()
class STEALTH_API UGuardNpcContextComponent : public UNpcContextComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Guard|State")
	int32 GlobalAlarmLevel = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Guard|Patrol")
	int32 CurrentPatrolIndex = 0;

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Guard|Patrol")
	UNpcPatrolComponent* GetPatrolComponent() const;

	UFUNCTION(BlueprintPure, Category = "Guard|Patrol")
	AActor* GetCurrentPatrolPoint() const;

	UFUNCTION(BlueprintCallable, Category = "Guard|Patrol")
	void IncrementPatrolIndex() const;

	UFUNCTION(BlueprintPure, Category = "Guard|Patrol")
	bool IsOnWalkingPatrol() const;

	UFUNCTION(BlueprintCallable, Category = "Guard|Actions")
	void LookAtPlayer();

private:
	UFUNCTION()
	void OnAlarmChanged(int32 NewLevel, const FVector& SourceLocation);

	UPROPERTY()
	mutable TObjectPtr<UNpcPatrolComponent> PatrolComponent;
};
