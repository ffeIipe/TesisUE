// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TesisUE : ModuleRules
{
	public TesisUE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"Niagara",
			"MediaAssets",
			"AdpcmAudioDecoder",
			"FieldSystemEngine",
			"Chaos",
			"GameplayTags",
			"GameplayTasks",
			"GameplayAbilities",
			"MotionWarping",
			"MotionTrajectory",
			"PoseSearch",
			"NetCore",
			"AnimGraphRuntime", 
			"AIModule",
			"NavigationSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"AnimGraphRuntime",
			"DeveloperSettings"
		});

        bUseUnity = true;
		
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
