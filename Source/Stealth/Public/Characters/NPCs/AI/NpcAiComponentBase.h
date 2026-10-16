#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NpcAiComponentBase.generated.h"

class ANpcAiController;
class APawn;
class UNpcProfile;

/**
 * Base actor component for NPC AI stack components (Sensor, Focus, Awareness, Context).
 * Resolves and caches references to the owning AI controller, pawn, and profile.
 */
UCLASS(Abstract, ClassGroup = (AI))
class STEALTH_API UNpcAiComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	UNpcAiComponentBase();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "AI|Component")
	ANpcAiController* GetNpcController() const;

	UFUNCTION(BlueprintPure, Category = "AI|Component")
	APawn* GetNpcPawn() const;

	UFUNCTION(BlueprintPure, Category = "AI|Component")
	virtual const UNpcProfile* GetNpcProfile() const;

protected:
	void ResolveOwnerReferences() const;

protected:
	mutable TWeakObjectPtr<ANpcAiController> CachedController;
	mutable TWeakObjectPtr<APawn> CachedPawn;
};
