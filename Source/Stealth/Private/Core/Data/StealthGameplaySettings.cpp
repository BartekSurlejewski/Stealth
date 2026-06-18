#include "Core/Data/StealthGameplaySettings.h"

const UStealthGameplaySettings* UStealthGameplaySettings::GetStealthGameplaySettings()
{
	return GetDefault<UStealthGameplaySettings>();
}
