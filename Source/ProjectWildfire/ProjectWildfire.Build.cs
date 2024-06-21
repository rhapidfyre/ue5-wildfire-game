// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectWildfire : ModuleRules
{
	public ProjectWildfire(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
	        "Core",
	        "CoreUObject",
	        "Engine",
	        "InputCore",
	        "NavigationSystem",
	        "AIModule",
	        "Niagara",
	        "ChaosVehicles"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
	        "Slate",
	        "SlateCore",
	        "UMG",
	        "EnhancedInput",
	        "InputCore",
	        "GameplayTags",
	        "OnlineSubsystem",
	        "OnlineSubsystemUtils"
        });
    }
}
