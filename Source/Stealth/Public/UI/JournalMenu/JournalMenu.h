#pragma once

#include "CoreMinimal.h"
#include "UI/Core/StealthUserWidget.h"
#include "JournalMenu.generated.h"

class UVerticalBox;

UCLASS()
class STEALTH_API UJournalMenu : public UStealthUserWidget
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
