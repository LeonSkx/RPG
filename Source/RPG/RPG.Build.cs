// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RPG : ModuleRules
{
	public RPG(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"AIModule", 
			"UMG", 
			"Slate", 
			"SlateCore", 
			"NavigationSystem", 
			"GameplayAbilities", 
			"GameplayTasks", 
			"GameplayTags", 
			"Niagara",
			"MotionWarping",
			"ModelViewViewModel"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"GameplayTags",
			"GameplayTasks"
		});

		// Dependências condicionais para navegação
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { 
				"UnrealEd",
				"ToolMenus",
				"EditorStyle",
				"EditorWidgets"
			});
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
		
	}
}
