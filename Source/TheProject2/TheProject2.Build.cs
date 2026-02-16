// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TheProject2 : ModuleRules
{
	public TheProject2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTags", "GameplayTasks"});

		PublicIncludePaths.AddRange(new string[] {
			"TheProject2",
			"TheProject2/Variant_Platforming",
			"TheProject2/Variant_Platforming/Animation",
			"TheProject2/Variant_Combat",
			"TheProject2/Variant_Combat/AI",
			"TheProject2/Variant_Combat/Animation",
			"TheProject2/Variant_Combat/Gameplay",
			"TheProject2/Variant_Combat/Interfaces",
			"TheProject2/Variant_Combat/UI",
			"TheProject2/Variant_SideScrolling",
			"TheProject2/Variant_SideScrolling/AI",
			"TheProject2/Variant_SideScrolling/Gameplay",
			"TheProject2/Variant_SideScrolling/Interfaces",
			"TheProject2/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
