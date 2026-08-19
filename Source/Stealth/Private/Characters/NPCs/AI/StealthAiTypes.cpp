#include "Characters/NPCs/AI/StealthAiTypes.h"

namespace StealthAiTags
{
	// NPC State Tags (Single Source of Truth)
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Unaware, "NPC.State.Unaware");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Suspicious, "NPC.State.Suspicious");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Alerted, "NPC.State.Alerted");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Combat, "NPC.State.Combat");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Search, "NPC.State.Search");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Dead, "NPC.State.Dead");

	// NPC Event / Trigger Tags
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_Investigate, "NPC.Event.Investigate");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_NoiseHeard, "NPC.Event.NoiseHeard");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_CrimeReported, "NPC.Event.CrimeReported");

	// Player Status Tags
	UE_DEFINE_GAMEPLAY_TAG(TAG_Player_State_Illegal, "Player.State.Illegal");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Player_State_Trespassing, "Player.State.Trespassing");
}
