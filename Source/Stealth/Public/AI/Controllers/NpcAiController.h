#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "NpcAiController.generated.h"

struct FGameplayTag;
class UStateTreeAIComponent;
class UPlayerExposureSubsystem;
struct FAIStimulus;
class UAIPerceptionComponent;

USTRUCT(BlueprintType)
struct FNpcCharacterState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	bool bSeesPlayer = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
	bool bIsAwareOfPlayer = false;
};

UCLASS()
class STEALTH_API ANpcAiController : public AAIController
{
	GENERATED_BODY()

public:
	ANpcAiController();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* TargetActor, FAIStimulus Stimulus);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	FGameplayTag ChangedStateEventTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTreeAIComponent> StateTreeAIComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="State", meta=(AllowPrivateAccess = "true"))
	FNpcCharacterState CharacterState;
	UPROPERTY()
	TObjectPtr<UPlayerExposureSubsystem> LightExposureSubsystem;
};
