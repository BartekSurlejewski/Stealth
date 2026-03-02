#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NpcCharacter.generated.h"

class ANpcAiController;

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

UCLASS(Abstract)
class STEALTH_API ANpcCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY();
	TObjectPtr<ANpcAiController> AiController;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State", meta=(AllowPrivateAccess = "true"))
	FNpcCharacterState CharacterState;

public:
	ANpcCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
