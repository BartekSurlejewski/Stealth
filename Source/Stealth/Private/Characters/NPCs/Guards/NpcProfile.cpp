#include "Characters/NPCs/Guards/NpcProfile.h"
#include "Characters/NPCs/AI/States/NpcState.h"

UNpcStateProfile* UNpcProfile::GetStateProfile(const FGameplayTag& StateTag) const
{
	if (const TObjectPtr<UNpcStateProfile>* FoundProfile = StateProfiles.Find(StateTag))
	{
		return *FoundProfile;
	}
	return nullptr;
}
