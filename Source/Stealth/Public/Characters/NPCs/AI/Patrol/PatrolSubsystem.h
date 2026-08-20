#pragma once

#include "CoreMinimal.h"
#include "Core/StealthWorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "PatrolSubsystem.generated.h"

class APatrolRoute;

/**
 * World Subsystem managing spatial registration and retrieval of Patrol Routes by LocationTag.
 */
UCLASS()
class STEALTH_API UPatrolSubsystem : public UStealthWorldSubsystem
{
	GENERATED_BODY()

public:
	static UPatrolSubsystem* Get(const UObject* WorldContextObject)
	{
		return UStealthWorldSubsystem::Get<UPatrolSubsystem>(WorldContextObject);
	}

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Register a patrol route actor in the world registry */
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	void RegisterPatrolRoute(APatrolRoute* Route);

	/** Unregister a patrol route actor */
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	void UnregisterPatrolRoute(APatrolRoute* Route);

	/** Find the best active patrol route matching the requested location tag */
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	APatrolRoute* GetPatrolRouteByLocationTag(const FGameplayTag& LocationTag, const FVector& RequesterLocation = FVector::ZeroVector) const;

	/** Get all active patrol routes matching the specified location tag */
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	TArray<APatrolRoute*> GetAllPatrolRoutesByLocationTag(const FGameplayTag& LocationTag) const;

	/** Get all registered patrol routes */
	UFUNCTION(BlueprintCallable, Category = "AI|Patrol")
	TArray<APatrolRoute*> GetAllRegisteredRoutes() const;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<APatrolRoute>> RegisteredRoutes;
};
