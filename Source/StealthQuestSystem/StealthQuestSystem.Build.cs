using UnrealBuildTool;

public class StealthQuestSystem : ModuleRules
{
	public StealthQuestSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayMessageRuntime"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
			{ "GameplayAbilities", "GameplayTags" });

		PublicIncludePaths.AddRange(new string[]
		{
			"StealthQuestSystem/Public"
		});
	}
}
