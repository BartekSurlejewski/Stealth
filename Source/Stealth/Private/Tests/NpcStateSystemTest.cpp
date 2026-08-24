#include "Misc/AutomationTest.h"
#include "Characters/NPCs/AI/Focus/NpcFocusComponent.h"
#include "Characters/NPCs/AI/States/NpcState.h"
#include "Characters/NPCs/AI/Suspicion/NpcSuspicionComponent.h"
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNpcStateOpenClosedExtensibilityTest, "Stealth.AI.State.OpenClosedExtensibility", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNpcStateOpenClosedExtensibilityTest::RunTest(const FString& Parameters)
{
	UNpcSuspicionComponent* SuspicionComp = NewObject<UNpcSuspicionComponent>();
	SuspicionComp->InitializeDefaultStates();

	// Verify UNpcState_Fleeing is registered and accessible via tag
	UNpcState* FleeingState = SuspicionComp->GetStateByTag(StealthAiTags::TAG_NPC_State_Fleeing);
	TestNotNull(TEXT("UNpcState_Fleeing should be retrieved by TAG_NPC_State_Fleeing"), FleeingState);

	const bool bTransitioned = SuspicionComp->TransitionToStateByTag(StealthAiTags::TAG_NPC_State_Fleeing);
	TestTrue(TEXT("Should transition cleanly to UNpcState_Fleeing"), bTransitioned);
	TestTrue(TEXT("Current state tag must be TAG_NPC_State_Fleeing"), SuspicionComp->GetCurrentStateTag().MatchesTagExact(StealthAiTags::TAG_NPC_State_Fleeing));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNpcLegalitySuspicionGatingTest, "Stealth.AI.Suspicion.LegalityGating", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNpcLegalitySuspicionGatingTest::RunTest(const FString& Parameters)
{
	UNpcState_Unaware* UnawareState = NewObject<UNpcState_Unaware>();

	// Context: Player is in direct sight and focused, but NOT performing illegal action
	FNpcStateTickContext LegalCtx;
	LegalCtx.DeltaTime = 0.05f;
	LegalCtx.bHasLineOfSight = true;
	LegalCtx.bIsFocusingOnPlayer = true;
	LegalCtx.bIsLookingDirectlyAtPlayer = true;
	LegalCtx.bIsPlayerPerformingIllegalAction = false;
	LegalCtx.BaseGainRateSight = 40.0f;
	LegalCtx.BaseDecayRate = 8.0f;

	const float LegalDelta = UnawareState->GetSuspicionDelta(LegalCtx);
	TestTrue(TEXT("Suspicion delta must not increase when player is not performing illegal action"), LegalDelta <= 0.0f);

	// Context: Player is performing illegal action
	FNpcStateTickContext IllegalCtx = LegalCtx;
	IllegalCtx.bIsPlayerPerformingIllegalAction = true;

	const float IllegalDelta = UnawareState->GetSuspicionDelta(IllegalCtx);
	TestTrue(TEXT("Suspicion delta must increase when player is performing illegal action"), IllegalDelta > 0.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNpcStateSingleSourceOfTruthTest, "Stealth.AI.State.SingleSourceOfTruth", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FNpcStateSingleSourceOfTruthTest::RunTest(const FString& Parameters)
{
	UNpcSuspicionComponent* SuspicionComp = NewObject<UNpcSuspicionComponent>();
	SuspicionComp->InitializeDefaultStates();

	// Default state is Unaware
	TestTrue(TEXT("Default AlertLevel must resolve from Unaware state"), SuspicionComp->GetAlertLevel() == ENpcAlertLevel::Unaware);
	TestTrue(TEXT("Default BehaviourState must resolve from Unaware state"), SuspicionComp->GetBehaviourState() == ENpcBehaviourState::Routine);

	// Transition to Combat
	SuspicionComp->TransitionToStateByTag(StealthAiTags::TAG_NPC_State_Combat);
	TestTrue(TEXT("AlertLevel must resolve to Hostile in Combat state"), SuspicionComp->GetAlertLevel() == ENpcAlertLevel::Hostile);
	TestTrue(TEXT("BehaviourState must resolve to Combat in Combat state"), SuspicionComp->GetBehaviourState() == ENpcBehaviourState::Combat);

	// Transition to Alerted
	SuspicionComp->TransitionToStateByTag(StealthAiTags::TAG_NPC_State_Alerted);
	TestTrue(TEXT("AlertLevel must resolve to Alerted in Alerted state"), SuspicionComp->GetAlertLevel() == ENpcAlertLevel::Alerted);
	TestTrue(TEXT("BehaviourState must resolve to Alerted in Alerted state"), SuspicionComp->GetBehaviourState() == ENpcBehaviourState::Alerted);

	return true;
}

#endif
