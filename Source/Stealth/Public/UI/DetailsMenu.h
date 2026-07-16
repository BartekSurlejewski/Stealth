#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Core/StealthUserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "DetailsMenu.generated.h"


struct FInputMessage;
struct FGameplayTag;
class UButton;
class UWidgetSwitcher;

USTRUCT(BlueprintType)
struct FSubmenuInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UButton> Button;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag Tag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Index;
};

UCLASS(Blueprintable)
class STEALTH_API UDetailsMenu : public UStealthUserWidget
{
	GENERATED_BODY()
	/*Methods*/
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	void Show(FGameplayTag SubmenuTag);

private:
	UFUNCTION()
	void OnPrevSubmenuInput(FGameplayTag Channel, const FInputMessage& Message);
	UFUNCTION()
	void OnNextSubmenuInput(FGameplayTag Channel, const FInputMessage& Message);

	/*Properties*/
protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> DailyRegimenButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> InventoryButton;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UButton> JournalButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Submenus")
	FSubmenuInfo DailyRegimenSubmenuInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Submenus")
	FSubmenuInfo InventorySubmenuInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stealth|Submenus")
	FSubmenuInfo JournalSubmenuInfo;

private:
	UPROPERTY()
	TArray<FSubmenuInfo> SubmenusInfo;

	FGameplayMessageListenerHandle PrevSubmenuInputListenerHandle;
	FGameplayMessageListenerHandle NextSubmenuInputListenerHandle;
};
