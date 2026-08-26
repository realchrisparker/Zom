// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Zom : ModuleRules
{
	public Zom(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"MotionWarping",
			"MotionTrajectory",
			"PhysicsCore",
			"AnimGraphRuntime",
			"AIModule",
			"PoseSearch",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"NavigationSystem",
			"GameplayAbilities",
			"GameplayTasks",
			"Niagara",
			"EngineCameras",
			"GameplayCameras",
			"GameplayTags",
			"AnimationWarpingRuntime",
			"Chooser",
			"PropertyPath",
			"DrawDebugLibrary",
			"CustomizableObject",
			"UMG"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
