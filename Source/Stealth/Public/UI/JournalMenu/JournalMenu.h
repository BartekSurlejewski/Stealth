#pragma once

#include "CoreMinimal.h"
#include "UI/Core/BaseWidgets/StealthBaseMenu.h"
#include "UI/Core/BaseWidgets/StealthBaseWidget.h"
#include "JournalMenu.generated.h"

class UVerticalBox;

UCLASS()
class STEALTH_API UJournalMenu : public UStealthBaseMenu
{
	GENERATED_BODY()
	/*Methods*/
protected:
	virtual void OnShow_Implementation() override;

	/*Properties*/
protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UVerticalBox> QuestDescriptionsContainer;
};
