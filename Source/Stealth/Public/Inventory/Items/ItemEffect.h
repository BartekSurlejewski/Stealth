#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemEffect.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class STEALTH_API UItemEffect : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent)
	void OnUse(AActor* InstigatorActor);

	UFUNCTION(BlueprintNativeEvent)
	void OnEquip(AActor* InstigatorActor);

	UFUNCTION(BlueprintNativeEvent)
	void OnUnequip(AActor* InstigatorActor);

	virtual void OnUse_Implementation(AActor* InstigatorActor) {}
	virtual void OnEquip_Implementation(AActor* InstigatorActor) {}
	virtual void OnUnequip_Implementation(AActor* InstigatorActor) {}
};
