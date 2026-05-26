#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StealthPlayerController.generated.h"

class UInputAction;
class UInventoryComponent;
class UInputMappingContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryInput);

UCLASS()
class STEALTH_API AStealthPlayerController : public APlayerController
{
	GENERATED_BODY()
	/*Events*/
public:
	UPROPERTY(BlueprintAssignable)
	FOnInventoryInput OnInventoryInput;

	/*Methods*/
public:
	AStealthPlayerController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	virtual void SetupInputComponent() override;
	UFUNCTION(Category="Input")
	void BindInputActions();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void OnOpenInventoryInput();

	/*Properties*/
protected:
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TObjectPtr<UInputMappingContext> MenuMappingContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(EditDefaultsOnly, Category ="Input")
	TObjectPtr<UInputAction> InventoryAction;

private:
	UPROPERTY()
	bool bIsInMenu = false;
};
