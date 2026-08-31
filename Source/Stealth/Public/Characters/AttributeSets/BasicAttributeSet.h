#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "BasicAttributeSet.generated.h"

UCLASS()
class STEALTH_API UBasicAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Health);
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Stamina);
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxStamina);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData SpeedCoefficient;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, SpeedCoefficient);
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData BaseCharacterSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, BaseCharacterSpeed);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData JumpVelocityCoefficient;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, JumpVelocityCoefficient);
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData BaseJumpVelocity;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, BaseJumpVelocity);

public:
	UBasicAttributeSet();

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
};
