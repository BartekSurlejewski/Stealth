#pragma once

#include "CoreMinimal.h"
#include "USpecialDialogueBehaviour.h"
#include "SpecialDialogueBehaviour_StartQuest.generated.h"

UCLASS()
class STEALTH_API USpecialDialogueBehaviour_StartQuest : public USpecialDialogueBehaviour
{
	GENERATED_BODY()

public:
	virtual void PerformAction_Implementation(const UObject* WorldContextObject) override;

protected:
	UPROPERTY(EditAnywhere, meta=(AllowedTypes="QuestDefinition"))
	FPrimaryAssetId QuestToStart;
};
