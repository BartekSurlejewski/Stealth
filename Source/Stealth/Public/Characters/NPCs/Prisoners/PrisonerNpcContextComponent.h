#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "PrisonerNpcContextComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEALTH_API UPrisonerNpcContextComponent : public UNpcContextComponent
{
	GENERATED_BODY()

public:
	UPrisonerNpcContextComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
