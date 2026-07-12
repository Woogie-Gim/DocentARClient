// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class DocentARClient : ModuleRules
{
	public DocentARClient(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"Sockets", 
			"Networking",
			"HeadMountedDisplay"
		});

        // 안드로이드 플랫폼 빌드 시 UPL 파일 포함 로직
        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            // JNI 및 안드로이드 기능 사용을 위한 Launch 모듈 의존성 추가
            PrivateDependencyModuleNames.Add("Launch");

            string UPLPath = Path.Combine(ModuleDirectory, "DocentARClient_UPL.xml");
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", UPLPath);
        }

        PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
