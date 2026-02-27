#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NpcCharacter.generated.h"

class ANpcAiController;

UCLASS(Abstract)
class STEALTH_API ANpcCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY();
	TObjectPtr<ANpcAiController> AiController;

public:
	ANpcCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
