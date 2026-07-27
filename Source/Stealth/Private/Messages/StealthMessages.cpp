#include "Messages/StealthMessages.h"

namespace StealthMessageChannels
{
	// Player
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Player_LookedAtInteractable, "Message.Player.LookedAtInteractable");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Player_IsInRestrictedAreaChanged, "Message.Player.IsInRestrictedAreaChanged");

	// Input
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Input_OpenWidget, "Message.Input.OpenWidget");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Input_CloseWidget, "Message.Input.CloseWidget");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Input_DetailsMenu_Next, "Message.Input.DetailsMenu.Next");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Input_DetailsMenu_Prev, "Message.Input.DetailsMenu.Prev");

	// Daily Tasks
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_DailyTask_OnDailyTaskStarted, "Message.DailyTask.OnDailyTaskStarted");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_DailyTask_OnDailyTaskEnded, "Message.DailyTask.OnDailyTaskEnded");

	// Time
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Time_TimeChanged, "Message.Time.TimeChanged");
	UE_DEFINE_GAMEPLAY_TAG(TAG_Message_Time_TimeOfDayChanged, "Message.Time.TimeOfDayChanged");
}
