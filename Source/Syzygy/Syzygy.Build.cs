// Copyright Aerelion Systems. All Rights Reserved.
// PROJECT: SYZYGY — Build configuration

using UnrealBuildTool;

public class Syzygy : ModuleRules
{
	public Syzygy(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTasks",
			"GameplayTags",
			"AIModule",
			"NavigationSystem",
			"UMG",
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
		});

		// Uncomment when Inworld AI plugin is installed
		// PublicDependencyModuleNames.Add("InworldAIIntegration");
	}
}
