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

			PublicAdditionalFrameworks.Add(new Framework("Sentry", Path.Combine(PlatformThirdPartyPath, "Sentry.embeddedframework.zip"), null, true));

			PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "Sentry_IOS_UPL.xml"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
			PublicDefinitions.Add("COCOAPODS=0");
			PublicDefinitions.Add("SENTRY_NO_UIKIT=1");
			PublicDefinitions.Add("APPLICATION_EXTENSION_API_ONLY_NO=0");
		}
		else if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Android"));

			PublicDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "Sentry_Android_UPL.xml"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicIncludePaths.Add(Path.Combine(PluginDirectory, "Source", "Sentry", "Private", "Microsoft"));
			
			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
			PublicDefinitions.Add("SENTRY_BUILD_STATIC=1");

			if (bBuildNativeForWindows)
			{
				PrivateDependencyModuleNames.Add("SentryNative");
			}
			else
			{
				bool CrashpadExists = File.Exists(Path.Combine(PlatformThirdPartyPath, "Crashpad", "bin", "crashpad_handler.exe"));

				string WindowsThirdPartyPath = Path.Combine(PlatformThirdPartyPath, CrashpadExists ? "Crashpad" : "Breakpad");

				PublicIncludePaths.Add(Path.Combine(WindowsThirdPartyPath, "include"));
				PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Windows"));

				PublicAdditionalLibraries.Add(Path.Combine(WindowsThirdPartyPath, "lib", "sentry.lib"));

				if (CrashpadExists)
				{
					RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "crashpad_handler.exe"), Path.Combine(WindowsThirdPartyPath, "bin", "crashpad_handler.exe"));

					PublicAdditionalLibraries.Add(Path.Combine(WindowsThirdPartyPath, "lib", "crashpad_compat.lib"));
					PublicAdditionalLibraries.Add(Path.Combine(WindowsThirdPartyPath, "lib", "crashpad_snapshot.lib"));
					PublicAdditionalLibraries.Add(Path.Combine(WindowsThirdPartyPath, "lib", "crashpad_util.lib"));
					PublicAdditionalLibraries.Add(Path.Combine(WindowsThirdPartyPath, "lib", "mini_chromium.lib"));
					PublicAdditionalLibraries.Add(Path.Combine(WindowsThirdPartyPath, "lib", "crashpad_client.lib"));
					PublicAdditionalLibraries.Add(Path.Combine(WindowsThirdPartyPath, "lib", "crashpad_zlib.lib"));
					PublicAdditionalLibraries.Add(Path.Combine(WindowsThirdPartyPath, "lib", "crashpad_getopt.lib"));
					PublicAdditionalLibraries.Add(Path.Combine(WindowsThirdPartyPath, "lib", "crashpad_tools.lib"));
					PublicAdditionalLibraries.Add(Path.Combine(WindowsThirdPartyPath, "lib", "crashpad_handler_lib.lib"));
				}
				else
				{
					PublicAdditionalLibraries.Add(Path.Combine(WindowsThirdPartyPath, "lib", "breakpad_client.lib"));
				}
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
			AddEngineThirdPartyPrivateStaticDependencies(Target, "libcurl");

			PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyPath, "include"));
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Linux"));

			RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "crashpad_handler"), Path.Combine(PlatformThirdPartyPath, "bin", "crashpad_handler"));

			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "lib", "libcrashpad_client.a"));
			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "lib", "libcrashpad_compat.a"));
			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "lib", "libcrashpad_handler_lib.a"));
			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "lib", "libcrashpad_minidump.a"));
			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "lib", "libcrashpad_snapshot.a"));
			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "lib", "libcrashpad_tools.a"));
			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "lib", "libcrashpad_util.a"));
			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "lib", "libmini_chromium.a"));
			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "lib", "libsentry.a"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
			PublicDefinitions.Add("SENTRY_BUILD_STATIC=1");
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyPath, "include"));
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));

			RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "sentry.dylib"), Path.Combine(PlatformThirdPartyPath, "bin", "sentry.dylib"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
			PublicDefinitions.Add("COCOAPODS=0");
			PublicDefinitions.Add("SENTRY_NO_UIKIT=1");
			PublicDefinitions.Add("APPLICATION_EXTENSION_API_ONLY_NO=0");
		}
		else if (Target.Platform.ToString() == "XSX")
		{
			PrivateDependencyModuleNames.Add("SentryNative");
		}
		else
		{
			Console.WriteLine("Sentry Unreal SDK does not support platform: " + Target.Platform);
		}
	}
}
