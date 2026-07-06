#pragma once

#include "CoreMinimal.h"
#include "Characters/NPCs/NpcAiController.h"
#include "PrisonerNpcAiController.generated.h"

class UPrisonerNpcContextComponent;

UCLASS()
class STEALTH_API APrisonerNpcAiController : public ANpcAiController
{
	GENERATED_BODY()

public:
	APrisonerNpcAiController();

protected:
	virtual void BeginPlay() override;
	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override;

	/*Properties*/
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPrisonerNpcContextComponent> PrisonerContext;
};
