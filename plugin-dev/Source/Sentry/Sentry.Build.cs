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
		
		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PrivateDependencyModuleNames.Add("SentryIOS");
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateDependencyModuleNames.Add("SentryAndroid");
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PrivateDependencyModuleNames.Add("SentryWindows");
		}
#if UE_5_0_OR_LATER
		else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxArm64)
#else
		else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxAArch64)
#endif
		{
			AddEngineThirdPartyPrivateStaticDependencies(Target, "libcurl");
			PrivateDependencyModuleNames.Add("SentryLinux");
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PrivateDependencyModuleNames.Add("SentryMac");
		}
		else if (Target.Platform.ToString() == "XSX")
		{
			PrivateDependencyModuleNames.Add("SentryXbox");
		}
		else
		{
			Console.WriteLine("Sentry Unreal SDK does not support platform: " + Target.Platform);
		}
	}
}
