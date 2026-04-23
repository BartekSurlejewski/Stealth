#pragma once

#include "CoreMinimal.h"
#include "BasicAttributeSet.h"
#include "StealthCharacterAttibuteSet.generated.h"


UCLASS()
class STEALTH_API UStealthCharacterAttributeSet : public UBasicAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", meta=(ToolTip="MaxSatiety - not hungry, 0 - extremely hungry"))
	FGameplayAttributeData Satiety;
	ATTRIBUTE_ACCESSORS_BASIC(UStealthCharacterAttributeSet, Satiety);
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxSatiety;
	ATTRIBUTE_ACCESSORS_BASIC(UStealthCharacterAttributeSet, MaxSatiety);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", meta=(ToolTip="MaxRest - not tired, 0 - extremely tired"))
	FGameplayAttributeData Rest;
	ATTRIBUTE_ACCESSORS_BASIC(UStealthCharacterAttributeSet, Rest);
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxRest;
	ATTRIBUTE_ACCESSORS_BASIC(UStealthCharacterAttributeSet, MaxRest);

public:
	UStealthCharacterAttributeSet();
};
