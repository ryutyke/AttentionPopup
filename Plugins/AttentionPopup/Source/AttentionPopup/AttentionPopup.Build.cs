// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AttentionPopup : ModuleRules
{
	public AttentionPopup(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"ApplicationCore",
                "EditorStyle",
                "DeveloperSettings",
                "InteractiveToolsFramework"
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
                "UnrealEd",
				"EditorFramework",
                "LevelEditor",
                "Projects",
                "ToolMenus",
                "EditorWidgets"
            }
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
