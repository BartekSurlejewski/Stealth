#include "Characters/NPCs/AI/Debug/NpcPerceptionDebugHelper.h"
#include "Characters/NPCs/AI/Perception/NpcAwarenessComponent.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

namespace NpcAiDebug
{
	void DrawPerceptionDebug(
		const UNpcAwarenessComponent* Awareness,
		const UNpcFocusComponent* Focus,
		const UNpcContextComponent* Context,
		const APawn* Pawn)
	{
		if (!Pawn || !Pawn->GetWorld() || !Awareness)
		{
			return;
		}

		const FNpcAwarenessState& State = Awareness->GetAwarenessState();
		const FString StateTagStr = Context ? Context->GetCurrentStateTag().ToString() : TEXT("None");
		const FString FocusTagStr = (Focus && Focus->GetCurrentFocus().IsValid()) ? Focus->GetCurrentFocus().FocusTag.ToString() : TEXT("None");
		const FString VisGradeStr = UEnum::GetValueAsString(State.VisibilityGrade);

		const FString DebugStr = FString::Printf(
			TEXT("NPC [%s]\nSuspicion: %.1f | VisGrade: %s | Stale: %s\nHasInvestLoc: %s | State: %s\nFocus: %s"),
			*Pawn->GetName(),
			State.Suspicion,
			*VisGradeStr,
			State.bFocusTargetStale ? TEXT("true") : TEXT("false"),
			State.bHasInvestigationLocation ? TEXT("true") : TEXT("false"),
			*StateTagStr,
			*FocusTagStr
		);

		DrawDebugString(Pawn->GetWorld(), Pawn->GetActorLocation() + FVector(0, 0, 120), DebugStr, nullptr, FColor::Yellow, 0.0f, true);
	}
}
