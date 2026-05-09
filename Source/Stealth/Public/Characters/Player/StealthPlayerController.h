#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StealthPlayerController.generated.h"

class UInputAction;
class UInventoryComponent;
class UInputMappingContext;

UCLASS()
class STEALTH_API AStealthPlayerController : public APlayerController
{
	GENERATED_BODY()
	/*Methods*/
public:
	AStealthPlayerController();

protected:
	virtual void SetupInputComponent() override;
	UFUNCTION(Category="Input")
	void BindInputActions();
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void OnOpenInventoryInput();

	/*Properties*/
protected:
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY(EditDefaultsOnly, Category ="Input")
	TObjectPtr<UInputAction> InventoryAction;
};
