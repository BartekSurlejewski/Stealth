#include "Characters/NPCs/Guards/NpcProfile.h"

UNpcProfile::UNpcProfile()
{
	// Noise Tags Severity Defaults
	NoiseTagSeverity.Add(TEXT("Explosion"), ENpcNoiseType::Critical);
	NoiseTagSeverity.Add(TEXT("DeathScream"), ENpcNoiseType::Critical);
	NoiseTagSeverity.Add(TEXT("CombatNoise"), ENpcNoiseType::Critical);
	NoiseTagSeverity.Add(TEXT("GlassShatter"), ENpcNoiseType::Major);
	NoiseTagSeverity.Add(TEXT("Alarm"), ENpcNoiseType::Major);
	NoiseTagSeverity.Add(TEXT("DoorForced"), ENpcNoiseType::Major);
	NoiseTagSeverity.Add(TEXT("Distraction"), ENpcNoiseType::Distraction);
	NoiseTagSeverity.Add(TEXT("RockThrow"), ENpcNoiseType::Distraction);
	NoiseTagSeverity.Add(TEXT("Whistle"), ENpcNoiseType::Distraction);

	// Noise Focus Rules
	{
		FNpcFocusRule DistractionRule;
		DistractionRule.FocusTag = StealthAiTags::TAG_NPC_Focus_Noise_Distraction;
		DistractionRule.Priority = ENpcFocusPriority::MinorDistraction;
		DistractionRule.Duration = 4.0f;
		DistractionRule.AwarenessReductionMultiplier = 0.5f;
		NoiseFocusRules.Add(ENpcNoiseType::Distraction, DistractionRule);
	}
	{
		FNpcFocusRule MajorRule;
		MajorRule.FocusTag = StealthAiTags::TAG_NPC_Focus_Disturbance_Environment;
		MajorRule.Priority = ENpcFocusPriority::MajorDisturbance;
		MajorRule.Duration = 5.0f;
		MajorRule.AwarenessReductionMultiplier = 0.3f;
		NoiseFocusRules.Add(ENpcNoiseType::Major, MajorRule);
	}
	{
		FNpcFocusRule CriticalRule;
		CriticalRule.FocusTag = StealthAiTags::TAG_NPC_Focus_Disturbance_DeadBody;
		CriticalRule.Priority = ENpcFocusPriority::CriticalDisturbance;
		CriticalRule.Duration = 6.0f;
		CriticalRule.AwarenessReductionMultiplier = 0.1f;
		NoiseFocusRules.Add(ENpcNoiseType::Critical, CriticalRule);
	}

	// Player Sighted Focus Rule
	PlayerSightedFocusRule.FocusTag = StealthAiTags::TAG_NPC_Focus_Player_Suspicious;
	PlayerSightedFocusRule.Priority = ENpcFocusPriority::SuspiciousPlayer;
	PlayerSightedFocusRule.Duration = 3.0f;
	PlayerSightedFocusRule.AwarenessReductionMultiplier = 1.0f;

	// Player Touched Focus Rule
	PlayerTouchedFocusRule.FocusTag = StealthAiTags::TAG_NPC_Focus_Player_Suspicious;
	PlayerTouchedFocusRule.Priority = ENpcFocusPriority::SuspiciousPlayer;
	PlayerTouchedFocusRule.Duration = 3.0f;
	PlayerTouchedFocusRule.AwarenessReductionMultiplier = 1.0f;

	// Player Damaged Focus Rule
	PlayerDamagedFocusRule.FocusTag = StealthAiTags::TAG_NPC_Focus_Player_Hostile;
	PlayerDamagedFocusRule.Priority = ENpcFocusPriority::CombatTarget;
	PlayerDamagedFocusRule.Duration = 0.0f; // Indefinite
	PlayerDamagedFocusRule.AwarenessReductionMultiplier = 1.0f;
}

ENpcNoiseType UNpcProfile::ClassifyNoise(const FName& StimulusTag, float Strength) const
{
	if (!StimulusTag.IsNone())
	{
		if (const ENpcNoiseType* Found = NoiseTagSeverity.Find(StimulusTag))
		{
			return *Found;
		}
	}

	if (Strength >= NoiseThreshold_Critical)
	{
		return ENpcNoiseType::Critical;
	}
	if (Strength >= NoiseThreshold_Major)
	{
		return ENpcNoiseType::Major;
	}
	if (Strength >= NoiseThreshold_Distraction)
	{
		return ENpcNoiseType::Distraction;
	}

	return ENpcNoiseType::Subtle;
}
