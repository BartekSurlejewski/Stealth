#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "NpcAiController.generated.h"

class UInventoryComponent;
class UNpcFocusComponent;
class UNpcSensorComponent;
class UNpcAwarenessComponent;
class UNpcContextComponent;
class UNpcProfile;
class UStateTreeAIComponent;

UCLASS(Abstract)
class STEALTH_API ANpcAiController : public AAIController
{
	GENERATED_BODY()

public:
	ANpcAiController();

	UFUNCTION(BlueprintPure, Category = "AI Components")
	UNpcFocusComponent* GetFocusComponent() const { return FocusComponent; }

	UFUNCTION(BlueprintPure, Category = "AI Components")
	UNpcSensorComponent* GetSensorComponent() const { return SensorComponent; }

	UFUNCTION(BlueprintPure, Category = "AI Components")
	UNpcAwarenessComponent* GetAwarenessComponent() const { return AwarenessComponent; }

	UFUNCTION(BlueprintPure, Category = "AI Components")
	UNpcContextComponent* GetContextComponent() const { return ContextComponent; }

	UFUNCTION(BlueprintPure, Category = "AI Components")
	const UNpcProfile* GetNpcProfile() const;

protected:
	virtual void PostInitializeComponents() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	virtual void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION()
	virtual void OnTargetPerceptionForgotten(AActor* Actor);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	TObjectPtr<UStateTreeAIComponent> StateTreeAIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	TObjectPtr<UInventoryComponent> NpcInventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	TObjectPtr<UNpcSensorComponent> SensorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	TObjectPtr<UNpcAwarenessComponent> AwarenessComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	TObjectPtr<UNpcFocusComponent> FocusComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
	TObjectPtr<UNpcContextComponent> ContextComponent;
};
