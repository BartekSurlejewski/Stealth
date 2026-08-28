#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace StealthPlayerTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_State_Illegal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_State_Trespassing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_State_Movement_Sprinting);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_State_Movement_Falling);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_State_Movement_Moving);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_State_Movement_Crouching);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Player_State_Stamina_Regenerating);
}
