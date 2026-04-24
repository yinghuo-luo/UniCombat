// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UniCombat : ModuleRules
{
	public UniCombat(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore",
			"EnhancedInput",
			"DeveloperSettings",
			"UMG",
			"GameplayTags",
			"GameplayTasks",
			"GameplayAbilities",
			"AIModule"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AnimGraphRuntime",
			"MoviePlayer",
			"Slate",
			"SlateCore"
		});

		PrivateIncludePaths.AddRange(new string[] {"UniCombat/"});
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
