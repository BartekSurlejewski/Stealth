#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "StealthWorldSettings.generated.h"

class ARuntimeDataContainer;

UCLASS()
class STEALTH_API AStealthWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	bool IsGameplayWorld() const { return bIsGameplayWorld; }
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stealth")
	TObjectPtr<ARuntimeDataContainer> SceneReferences;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stealth")
	bool bIsGameplayWorld;
};
