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
	UFUNCTION()
	void Show();
	UFUNCTION()
	void Hide();

	FGameplayTag GetWidgetGameplayTag() const { return WidgetGameplayTag; }
	bool GetPauseGameOnShow() const { return bPauseGameOnShow; }

protected:
	UFUNCTION(BlueprintNativeEvent)
	void OnShow();
	UFUNCTION(BlueprintNativeEvent)
	void OnHide();

	virtual void OnShow_Implementation() {}
	virtual void OnHide_Implementation() {}

	/*Properties*/
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stealth")
	FGameplayTag WidgetGameplayTag;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stealth")
	bool bPauseGameOnShow = false;
};
