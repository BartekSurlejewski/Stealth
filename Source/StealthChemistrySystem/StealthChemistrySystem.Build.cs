using UnrealBuildTool;

public class StealthChemistrySystem : ModuleRules
{
	public StealthChemistrySystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayMessageRuntime",
			"GameplayTags",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"GameplayAbilities"
		});

		PublicIncludePaths.AddRange(new string[]
		{
			"StealthChemistrySystem/Public"
		});
	}
}
