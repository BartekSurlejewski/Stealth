#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StealthCharacterCollisionsComponent.generated.h"


class AStealthPlayerState;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UStealthCharacterCollisionsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStealthCharacterCollisionsComponent();
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnBeginOverlap(AActor* OtherActor);
	UFUNCTION()
	void OnEndOverlap(AActor* OtherActor);

private:
	UPROPERTY()
	TObjectPtr<AStealthPlayerState> PlayerState;
	UPROPERTY()
	int IllegalAreaCollidersOverlapCount = 0;
};
