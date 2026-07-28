#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Messages/StealthMessages.h"
#include "StealthPlayerState.generated.h"

class AStealthCharacter;
struct FInventoryItem;
class UInventoryComponent;

UCLASS()
class STEALTH_API AStealthPlayerState : public APlayerState
{
	GENERATED_BODY()

	/*Properties*/
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> InventoryComponent;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "PlayerState")
	bool bIsInRestrictedArea = false;

protected:
	UPROPERTY()
	TObjectPtr<AStealthCharacter> PlayerCharacter;

	/*Methods*/
public:
	AStealthPlayerState();

	[[nodiscard]] const TObjectPtr<UInventoryComponent>& GetInventoryComponent() const { return InventoryComponent; }
	[[nodiscard]] bool GetIsInRestrictedArea() const { return bIsInRestrictedArea; }

	UFUNCTION()
	void SetIsInRestrictedArea(bool newIsInRestrictedArea);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//Event Handlers
	UFUNCTION()
	void OnPlayerInventoryItemRemoved(FGameplayTag Channel, const FPlayerInventoryItemRemovedMessage& Message);

	/*Properties*/
private:
	FGameplayMessageListenerHandle PlayerInventoryItemRemovedListenerHandle;
};
