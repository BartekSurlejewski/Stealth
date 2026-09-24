#include "Misc/AutomationTest.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/AI/Perception/NpcSensorComponent.h"
#include "Characters/NPCs/AI/Perception/NpcAwarenessComponent.h"
#include "Characters/NPCs/NpcContextComponent.h"
#include "Characters/NPCs/AI/StealthAiTypes.h"
#include "Perception/AIPerceptionTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNpcFocusIndefinitePreemptionTest, "Stealth.AI.Focus.IndefinitePreemptionProtection", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNpcFocusIndefinitePreemptionTest::RunTest(const FString& Parameters)
{
	UNpcFocusComponent* FocusComp = NewObject<UNpcFocusComponent>();

	// 1. Set indefinite combat target focus (Duration = 0.0f)
	FNpcFocusTarget CombatFocus;
	CombatFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Player_Hostile;
	CombatFocus.Priority = ENpcFocusPriority::CombatTarget; // 60
	CombatFocus.Duration = 0.0f; // Indefinite

	const bool bCombatAccepted = FocusComp->RequestFocus(CombatFocus);
	TestTrue(TEXT("Combat target focus should be accepted"), bCombatAccepted);
	TestTrue(TEXT("Current focus priority should be CombatTarget"), FocusComp->GetCurrentFocus().Priority == ENpcFocusPriority::CombatTarget);

	// 2. Attempt to preempt with a lower-priority distraction (e.g. MinorDistraction = 20 or MajorDisturbance = 30)
	FNpcFocusTarget MinorNoiseFocus;
	MinorNoiseFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Noise_Distraction;
	MinorNoiseFocus.Priority = ENpcFocusPriority::MinorDistraction; // 20
	MinorNoiseFocus.Duration = 4.0f;

	const bool bMinorNoiseAccepted = FocusComp->RequestFocus(MinorNoiseFocus);
	TestFalse(TEXT("Indefinite combat focus must NOT be preempted by MinorDistraction"), bMinorNoiseAccepted);
	TestTrue(TEXT("Focus must remain CombatTarget"), FocusComp->GetCurrentFocus().Priority == ENpcFocusPriority::CombatTarget);

	FNpcFocusTarget MajorDisturbanceFocus;
	MajorDisturbanceFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Disturbance_Environment;
	MajorDisturbanceFocus.Priority = ENpcFocusPriority::MajorDisturbance; // 30
	MajorDisturbanceFocus.Duration = 5.0f;

	const bool bMajorDisturbanceAccepted = FocusComp->RequestFocus(MajorDisturbanceFocus);
	TestFalse(TEXT("Indefinite combat focus must NOT be preempted by MajorDisturbance"), bMajorDisturbanceAccepted);
	TestTrue(TEXT("Focus must remain CombatTarget"), FocusComp->GetCurrentFocus().Priority == ENpcFocusPriority::CombatTarget);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNpcPlayerAttentionGatingTest, "Stealth.AI.Focus.PlayerAttentionGating", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNpcPlayerAttentionGatingTest::RunTest(const FString& Parameters)
{
	UNpcFocusComponent* FocusComp = NewObject<UNpcFocusComponent>();

	// 1. NPC is investigating a MajorDisturbance (Priority = 30)
	FNpcFocusTarget DisturbanceFocus;
	DisturbanceFocus.FocusLocation = FVector(100.0f, 200.0f, 0.0f);
	DisturbanceFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Disturbance_Environment;
	DisturbanceFocus.Priority = ENpcFocusPriority::MajorDisturbance; // 30
	DisturbanceFocus.Duration = 5.0f;

	const bool bDisturbanceAccepted = FocusComp->RequestFocus(DisturbanceFocus);
	TestTrue(TEXT("Major disturbance focus accepted"), bDisturbanceAccepted);

	// 2. Suspicious player candidate (Priority = 25) attempts to gain attention
	FNpcFocusTarget SuspiciousPlayerFocus;
	SuspiciousPlayerFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Player_Suspicious;
	SuspiciousPlayerFocus.Priority = ENpcFocusPriority::SuspiciousPlayer; // 25
	SuspiciousPlayerFocus.Duration = 3.0f;

	const bool bSuspiciousPlayerWon = FocusComp->RequestFocus(SuspiciousPlayerFocus);
	TestFalse(TEXT("Merely suspicious player must lose arbitration against active MajorDisturbance"), bSuspiciousPlayerWon);
	TestTrue(TEXT("NPC remains focused on MajorDisturbance"), FocusComp->GetCurrentFocus().Priority == ENpcFocusPriority::MajorDisturbance);

	// 3. Player commits an illegal action -> CombatTarget priority (60)
	FNpcFocusTarget IllegalPlayerFocus;
	IllegalPlayerFocus.FocusTag = StealthAiTags::TAG_NPC_Focus_Player_Hostile;
	IllegalPlayerFocus.Priority = ENpcFocusPriority::CombatTarget; // 60
	IllegalPlayerFocus.Duration = 0.0f;

	const bool bIllegalPlayerWon = FocusComp->RequestFocus(IllegalPlayerFocus);
	TestTrue(TEXT("Illegal player action (CombatTarget) beats MajorDisturbance in arbitration"), bIllegalPlayerWon);
	TestTrue(TEXT("NPC successfully switched focus to CombatTarget"), FocusComp->GetCurrentFocus().Priority == ENpcFocusPriority::CombatTarget);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNpcSuspicionModesTest, "Stealth.AI.Awareness.SuspicionModes", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNpcSuspicionModesTest::RunTest(const FString& Parameters)
{
	UNpcAwarenessComponent* AwarenessComp = NewObject<UNpcAwarenessComponent>();

	// Test SetPerceptionTuningDirect
	AwarenessComp->SetPerceptionTuningDirect(ENpcSuspicionMode::Normal, 1.2f, 0.8f);
	TestTrue(TEXT("Tuning mode should be Normal"), AwarenessComp->GetTuningParams().Mode == ENpcSuspicionMode::Normal);
	TestEqual(TEXT("Gain multiplier should be 1.2"), AwarenessComp->GetTuningParams().SuspicionGainMultiplier, 1.2f);
	TestEqual(TEXT("Decay multiplier should be 0.8"), AwarenessComp->GetTuningParams().SuspicionDecayMultiplier, 0.8f);

	// Test SetSuspicion clamp
	AwarenessComp->SetSuspicion(150.0f);
	TestEqual(TEXT("Suspicion clamped to 100"), AwarenessComp->GetSuspicion(), 100.0f);

	AwarenessComp->SetSuspicion(-50.0f);
	TestEqual(TEXT("Suspicion clamped to 0"), AwarenessComp->GetSuspicion(), 0.0f);

	AwarenessComp->AddSuspicion(50.0f);
	TestEqual(TEXT("Suspicion after add should be 50"), AwarenessComp->GetSuspicion(), 50.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNpcContextStateMappingTest, "Stealth.AI.Context.StateMapping", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNpcContextStateMappingTest::RunTest(const FString& Parameters)
{
	UNpcContextComponent* ContextComp = NewObject<UNpcContextComponent>();

	// Default state is Unaware
	ContextComp->SetCurrentStateTag(StealthAiTags::TAG_NPC_State_Unaware);
	TestTrue(TEXT("Default AlertLevel must resolve to Unaware"), ContextComp->GetAlertLevel() == ENpcAlertLevel::Unaware);
	TestTrue(TEXT("Default BehaviourState must resolve to Routine"), ContextComp->GetBehaviourState() == ENpcBehaviourState::Routine);

	// Combat State
	ContextComp->SetCurrentStateTag(StealthAiTags::TAG_NPC_State_Combat);
	TestTrue(TEXT("AlertLevel must resolve to Hostile in Combat state"), ContextComp->GetAlertLevel() == ENpcAlertLevel::Hostile);
	TestTrue(TEXT("BehaviourState must resolve to Combat in Combat state"), ContextComp->GetBehaviourState() == ENpcBehaviourState::Combat);

	// Alerted State
	ContextComp->SetCurrentStateTag(StealthAiTags::TAG_NPC_State_Alerted);
	TestTrue(TEXT("AlertLevel must resolve to Alerted in Alerted state"), ContextComp->GetAlertLevel() == ENpcAlertLevel::Alerted);
	TestTrue(TEXT("BehaviourState must resolve to Alerted in Alerted state"), ContextComp->GetBehaviourState() == ENpcBehaviourState::Alerted);

	// Search State
	ContextComp->SetCurrentStateTag(StealthAiTags::TAG_NPC_State_Search);
	TestTrue(TEXT("AlertLevel must resolve to Alerted in Search state"), ContextComp->GetAlertLevel() == ENpcAlertLevel::Alerted);
	TestTrue(TEXT("BehaviourState must resolve to Search in Search state"), ContextComp->GetBehaviourState() == ENpcBehaviourState::Search);

	// Suspicious State
	ContextComp->SetCurrentStateTag(StealthAiTags::TAG_NPC_State_Suspicious);
	TestTrue(TEXT("AlertLevel must resolve to Suspicious in Suspicious state"), ContextComp->GetAlertLevel() == ENpcAlertLevel::Suspicious);
	TestTrue(TEXT("BehaviourState must resolve to Suspicious in Suspicious state"), ContextComp->GetBehaviourState() == ENpcBehaviourState::Suspicious);

	return true;
}

#endif
