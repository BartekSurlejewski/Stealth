#pragma once

#include "CoreMinimal.h"

class UNpcAwarenessComponent;
class UNpcFocusComponent;
class UNpcContextComponent;
class APawn;

namespace NpcAiDebug
{
	void DrawPerceptionDebug(
		const UNpcAwarenessComponent* Awareness,
		const UNpcFocusComponent* Focus,
		const UNpcContextComponent* Context,
		const APawn* Pawn
	);
}
