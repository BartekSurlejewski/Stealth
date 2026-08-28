#include "Characters/AttributeSets/BasicAttributeSet.h"

#include "GameplayEffectExtension.h"

UBasicAttributeSet::UBasicAttributeSet()
{
	Health = 100;
	MaxHealth = 100;
	Stamina = 100;
	MaxStamina = 100;
	SpeedCoefficient = 1.0f;
	BaseCharacterSpeed = 800.0f;
}

void UBasicAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.f, GetMaxStamina()));
	}
	else if (Data.EvaluatedData.Attribute == GetSpeedCoefficientAttribute())
	{
		SetSpeedCoefficient(FMath::Max(0.f, GetSpeedCoefficient()));
	}
}
