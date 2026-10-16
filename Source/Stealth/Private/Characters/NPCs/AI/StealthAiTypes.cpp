#include "Characters/NPCs/AI/StealthAiTypes.h"

namespace StealthAiTags
{
	// NPC State Tags
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Unaware, "NPC.State.Unaware");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Suspicious, "NPC.State.Suspicious");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Alerted, "NPC.State.Alerted");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Combat, "NPC.State.Combat");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Search, "NPC.State.Search");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Dead, "NPC.State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_State_Fleeing, "NPC.State.Fleeing");

	// NPC Activity Tags
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Activity_Patrol, "NPC.Activity.Patrol");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Activity_GuardPost, "NPC.Activity.GuardPost");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Activity_Pub, "NPC.Activity.Pub");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Activity_Sleep, "NPC.Activity.Sleep");

	// NPC Focus Tags
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_None, "NPC.Focus.None");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Routine, "NPC.Focus.Routine");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Noise_Distraction, "NPC.Focus.Noise.Distraction");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Disturbance_Environment, "NPC.Focus.Disturbance.Environment");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Disturbance_DeadBody, "NPC.Focus.Disturbance.DeadBody");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Player, "NPC.Focus.Player");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Player_Suspicious, "NPC.Focus.Player.Suspicious");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Focus_Player_Hostile, "NPC.Focus.Player.Hostile");

	// NPC StateTree Event Tags
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_Investigate, "NPC.Event.Investigate");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_NoiseHeard, "NPC.Event.NoiseHeard");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_CrimeReported, "NPC.Event.CrimeReported");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_PlayerSpotted, "NPC.Event.PlayerSpotted");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_PlayerLost, "NPC.Event.PlayerLost");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_Distraction, "NPC.Event.Distraction");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_FocusCleared, "NPC.Event.FocusCleared");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_SearchExpired, "NPC.Event.SearchExpired");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_ResumeRoutine, "NPC.Event.ResumeRoutine");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_DamageReceived, "NPC.Event.DamageReceived");
	UE_DEFINE_GAMEPLAY_TAG(TAG_NPC_Event_TouchDetected, "NPC.Event.TouchDetected");
}
