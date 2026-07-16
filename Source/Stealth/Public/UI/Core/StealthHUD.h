#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/HUD.h"
#include "StealthHUD.generated.h"

struct FGameplayTagMessage;
class UStealthUserWidget;
class UUserWidget;

UCLASS()
class STEALTH_API AStealthHUD : public AHUD
{
	GENERATED_BODY()

	/*Methods*/
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowMainHUD();
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void CreateOptionalHUD();

private:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	UStealthUserWidget* ShowWidget(FGameplayTag WidgetTag, int32 ZOrder = 0);
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideWidget(FGameplayTag WidgetTag);
	UFUNCTION()
	UStealthUserWidget* CreateAndShowWidget(TSubclassOf<UStealthUserWidget> WidgetClass, bool bShow, int32 ZOrder = 0);
	UFUNCTION()
	void RemoveAndClearWidget(UStealthUserWidget* InstanceRef);

	void OnWidgetOpenMessage(FGameplayTag Channel, const FGameplayTagMessage& Message);
	void OnWidgetCloseMessage(FGameplayTag Channel, const FGameplayTagMessage& Message);

	/*Properties*/
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Widgets")
	TArray<TSubclassOf<UStealthUserWidget>> MainHUDWidgetClasses;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Widgets")
	TArray<TSubclassOf<UStealthUserWidget>> OptionalHUDWidgetClasses;

	// --- Live widget instances ---

	UPROPERTY(BlueprintReadOnly, Category = "HUD|Widgets")
	TMap<FGameplayTag, TObjectPtr<UStealthUserWidget>> ExistingWidgetsByTag;

private:
	FGameplayMessageListenerHandle WidgetOpenListenerHandle;
	FGameplayMessageListenerHandle WidgetCloseListenerHandle;
};
