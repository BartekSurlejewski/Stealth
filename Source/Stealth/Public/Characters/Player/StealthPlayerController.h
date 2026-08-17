#pragma once

#include "CoreMinimal.h"
#include "StealthPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/Items/InventoryItem.h"
#include "StealthPlayerController.generated.h"

class AStealthHUD;
struct FGameplayTag;
class AStealthPlayerCharacter;
class UInputAction;
class UInputMappingContext;


UCLASS()
class STEALTH_API AStealthPlayerController : public APlayerController
{
	GENERATED_BODY()

	/*Methods*/
public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;
	UFUNCTION(Category="Input")
	void BindInputActions();

	UFUNCTION(Category="Input")
	virtual void ToggleDetailsMenu(const FGameplayTag SubmenuTag);
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void OnToggleInventoryInput();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void OnToggleDetailsMenuInput();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void OnToggleJournalMenuInput();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void OnNextDetailsMenuInput();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void OnPrevDetailsMenuInput();

private:
	UFUNCTION()
	void OnWidgetShown(FGameplayTag Channel, const FWidgetToggleMessage& Message);
	UFUNCTION()
	void OnWidgetHidden(FGameplayTag Channel, const FWidgetToggleMessage& Message);

	/*Properties*/
protected:
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TObjectPtr<UInputMappingContext> MenuMappingContext;

	UPROPERTY(EditDefaultsOnly, Category ="Input|Actions")
	TObjectPtr<UInputAction> ToggleInventoryAction;
	UPROPERTY(EditDefaultsOnly, Category ="Input|Actions")
	TObjectPtr<UInputAction> ToggleDetailsMenuAction;
	UPROPERTY(EditDefaultsOnly, Category ="Input|Actions")
	TObjectPtr<UInputAction> ToggleJournalMenuAction;
	UPROPERTY(EditDefaultsOnly, Category ="Input|Actions|UI")
	TObjectPtr<UInputAction> NextDetailsMenuAction;
	UPROPERTY(EditDefaultsOnly, Category ="Input|Actions|UI")
	TObjectPtr<UInputAction> PrevDetailsMenuAction;

private:
	UPROPERTY()
	TObjectPtr<AStealthPlayerState> StealthPlayerState;
	UPROPERTY()
	TObjectPtr<AStealthHUD> HUD;

	FGameplayMessageListenerHandle OnWidgetShownHandle;
	FGameplayMessageListenerHandle OnWidgetHiddenHandle;
};
