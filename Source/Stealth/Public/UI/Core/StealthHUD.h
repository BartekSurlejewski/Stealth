#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "StealthUIData.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/HUD.h"
#include "StealthHUD.generated.h"

struct FGameplayTagMessage;
class UStealthBaseWidget;
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
	UFUNCTION(BlueprintCallable, Category = "HUD")
	bool IsAnyWidgetVisible(EUILayer Layer) const;

private:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	UStealthBaseWidget* ShowWidget(const FGameplayTag& WidgetTag);
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void HideWidget(const FGameplayTag& WidgetTag);
	UFUNCTION()
	UStealthBaseWidget* CreateWidgetInstance(TSubclassOf<UStealthBaseWidget> WidgetClass, bool bShow, int32 ZOrder = 0);
	UFUNCTION()
	void RemoveAndClearWidget(UStealthBaseWidget* InstanceRef);

	void OnWidgetToggleMessage(FGameplayTag Channel, const FGameplayTagMessage& Message);

	/*Properties*/
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Widgets")
	TArray<TSubclassOf<UStealthBaseWidget>> MainHUDWidgetClasses;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD|Widgets")
	TArray<TSubclassOf<UStealthBaseWidget>> OptionalHUDWidgetClasses;

	// --- Live widget instances ---

	UPROPERTY(BlueprintReadOnly, Category = "HUD|Widgets")
	TMap<FGameplayTag, TObjectPtr<UStealthBaseWidget>> ExistingWidgetsByTag;
	UPROPERTY(BlueprintReadOnly, Category = "HUD|Widgets")
	TSet<FGameplayTag> VisibleWidgetTags;
	UPROPERTY(BlueprintReadOnly, Category = "HUD|Widgets")
	TArray<TObjectPtr<UStealthBaseWidget>> VisibleWidgets;
	UPROPERTY(BlueprintReadOnly, Category = "HUD|Widgets")
	TMap<EUILayer, int32> VisibleWidgetsCountByLayer;

	static constexpr int32 MAX_VISIBLE_WIDGETS = 100;

private:
	FGameplayMessageListenerHandle WidgetToggleListenerHandle;
};

namespace StealthUiTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_DetailsMenu);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_DetailsMenu_Inventory);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_DetailsMenu_Journal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_DetailsMenu_DailyRegimen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_DialogueMenu);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_UI_FinishGameMenu);
}
