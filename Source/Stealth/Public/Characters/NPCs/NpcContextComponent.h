#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/StateTreeAIComponent.h"
#include "NpcContextComponent.generated.h"


class ANpcAiController;

UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UNpcContextComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcContextComponent();
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	FVector LastKnownPlayerPos = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	FVector LastHeardSoundLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	bool bPlayerInDirectSight = false;

	UPROPERTY(BlueprintReadOnly, Category="Guard|State")
	bool bPlayerInPeripheralSight = false;

protected:
	UPROPERTY()
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent;
	UPROPERTY()
	TObjectPtr<ANpcAiController> NpcAiController;
};
