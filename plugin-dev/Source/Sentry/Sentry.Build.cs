// Copyright (c) 2022 Sentry. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;
using System.Reflection;
using System.Diagnostics;
using System.Text;
using System.Collections.Generic;
using System.Text.RegularExpressions;

#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

public class Sentry : ModuleRules
{
	[CommandLine("-buildNativeForWindows")]
	public bool bBuildNativeForWindows = false;
	
	public Sentry(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Projects",
				"Json",
				"HTTP"
				// ... add private dependencies that you statically link with here ...	
			}
		);
		
		string PlatformThirdPartyPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Source", "ThirdParty", Target.Platform.ToString()));
		string PlatformBinariesPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Binaries", Target.Platform.ToString()));
		
		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));
			PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
			PrivateDependencyModuleNames.Add("SentryIOS");

		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Android"));

			PrivateDependencyModuleNames.Add("SentryAndroid");
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Microsoft"));

			if (bBuildNativeForWindows)
			{
				PrivateDependencyModuleNames.Add("SentryWin64Native");
			}
			else
			{
				PrivateDependencyModuleNames.Add("SentryWin64");
			}

			PublicSystemLibraries.Add("winhttp.lib");
			PublicSystemLibraries.Add("version.lib");
		}
#if UE_5_0_OR_LATER
		else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxArm64)
#else
		else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxAArch64)
#endif
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Linux"));
			AddEngineThirdPartyPrivateStaticDependencies(Target, "libcurl");

			PrivateDependencyModuleNames.Add("SentryLinux");
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));
			
			PrivateDependencyModuleNames.Add("SentryMac");
		}
		else
		{
			Console.WriteLine("Sentry Unreal SDK does not support platform: " + Target.Platform);
		}
	}
}
