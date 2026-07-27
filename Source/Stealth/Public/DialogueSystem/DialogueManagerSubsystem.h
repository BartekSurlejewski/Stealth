#pragma once

#include "CoreMinimal.h"
#include "Core/StealthWorldSubsystem.h"
#include "DialogueManagerSubsystem.generated.h"

UCLASS()
class STEALTH_API UDialogueManagerSubsystem : public UStealthWorldSubsystem
{
	GENERATED_BODY()

public:
	static UDialogueManagerSubsystem* Get(const UObject* WorldContextObject)
	{
		return UStealthWorldSubsystem::Get<UDialogueManagerSubsystem>(WorldContextObject);
	}
};
