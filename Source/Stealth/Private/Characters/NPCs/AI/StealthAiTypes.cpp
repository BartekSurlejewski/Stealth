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

	// NPC Activity Tags (Schedule & Routines)
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Activity_Patrol, "NPC.Activity.Patrol");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Activity_GuardPost, "NPC.Activity.GuardPost");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Activity_Pub, "NPC.Activity.Pub");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Activity_Sleep, "NPC.Activity.Sleep");

	// NPC Location Tags
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Location_Default, "NPC.Location.Default");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Location_Courtyard, "NPC.Location.Courtyard");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Location_Gate, "NPC.Location.Gate");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Location_Prison, "NPC.Location.Prison");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Location_Tavern, "NPC.Location.Tavern");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Location_Barracks, "NPC.Location.Barracks");

	// NPC Focus Tags
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_None, "NPC.Focus.None");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Routine, "NPC.Focus.Routine");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Noise_Distraction, "NPC.Focus.Noise.Distraction");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Disturbance_Environment, "NPC.Focus.Disturbance.Environment");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Disturbance_DeadBody, "NPC.Focus.Disturbance.DeadBody");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Player_Suspicious, "NPC.Focus.Player.Suspicious");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Player_Hostile, "NPC.Focus.Player.Hostile");
}
