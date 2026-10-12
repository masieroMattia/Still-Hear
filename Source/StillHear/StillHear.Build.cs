using UnrealBuildTool;

public class StillHear : ModuleRules
{
	public StillHear(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"EngineCameras",
			"Niagara", 
			"AIModule",
			"AudioMixer",
			"DeveloperSettings",
			"GameplayAbilities",
			"GameplayTasks", 
			"GameplayTags",
			"AnimGraphRuntime",
			"Flow",
			"InputDevice",
			"ApplicationCore",
			"GeometryCollectionEngine",
			"ChaosCaching",
			"ChaosSolverEngine",
			"PhysicsCore",
			"CommonUI",
			"CommonInput",
			"UMG",
			"Slate",
			"SlateCore",
			"PCG",
			"RenderCore",
			"RHI",
			"LevelSequence",
			"MovieScene"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"NavigationSystem", 
			"SignalProcessing", 
			"MotionWarping", 
			"DLSSBlueprint",
			"StreamlineDLSSGBlueprint",
			"StreamlineReflexBlueprint"
		});
		
		if (Target.bBuildEditor)
		{
			PublicDependencyModuleNames.AddRange(new string[]{
				"EditorStyle",
				"PropertyEditor",
				"UnrealEd",
				"EditorScriptingUtilities",
				"PCGEditor",
				"LevelEditor"
			});
		}

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
