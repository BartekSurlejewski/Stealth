using UnrealBuildTool;

public class StealthQuestSystemEditor : ModuleRules
{
	public StealthQuestSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AssetTools",
			"UnrealEd",
			"StealthQuestSystem",
			"EngineAssetDefinitions", "AssetDefinition"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
			{ });


		PublicIncludePaths.AddRange(new string[]
		{
			"StealthQuestSystemEditor/Public"
		});
	}
}
