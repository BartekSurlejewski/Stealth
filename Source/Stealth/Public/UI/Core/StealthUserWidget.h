#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "StealthUserWidget.generated.h"

UCLASS(Abstract, Blueprintable)
class STEALTH_API UStealthUserWidget : public UUserWidget
{
	GENERATED_BODY()
	/*Methods*/
public:
	UFUNCTION(BlueprintNativeEvent)
	void OnShow();
	UFUNCTION(BlueprintNativeEvent)
	void OnHide();

	virtual void OnShow_Implementation() {}
	virtual void OnHide_Implementation() {}

	FGameplayTag GetWidgetGameplayTag() const { return WidgetGameplayTag; }

	/*Properties*/
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stealth")
	FGameplayTag WidgetGameplayTag;
};
