#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "NpcAiController.generated.h"

class UNpcScheduleComponent;
class UInventoryComponent;
class UNpcFocusComponent;
class UNpcSuspicionComponent;
struct FGameplayTag;
class UStateTreeAIComponent;
class UPlayerExposureSubsystem;
struct FAIStimulus;
class UAIPerceptionComponent;

UCLASS(Abstract)
class STEALTH_API ANpcAiController : public AAIController
{
	GENERATED_BODY()

public:
	ANpcAiController();

	UFUNCTION(BlueprintPure, Category = "AI Components")
	UNpcFocusComponent* GetFocusComponent() const { return FocusComponent; }

	UFUNCTION(BlueprintPure, Category = "AI Components")
	UNpcSuspicionComponent* GetSuspicionComponent() const { return SuspicionComponent; }

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStateTreeAIComponent> StateTreeAIComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> NpcInventoryComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNpcScheduleComponent> ScheduleComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNpcSuspicionComponent> SuspicionComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UNpcFocusComponent> FocusComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI")
	FGameplayTag ChangedStateEventTag;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Identification")
	FGuid NpcGUID;
};
