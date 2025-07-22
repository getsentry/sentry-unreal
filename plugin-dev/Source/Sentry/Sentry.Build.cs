// Copyright (c) 2025 Sentry. All Rights Reserved.

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

		string PlatformThirdPartyPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Source", "ThirdParty", Target.Platform.ToString()));
		string PlatformBinariesPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Binaries", Target.Platform.ToString()));

		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));

			PublicAdditionalFrameworks.Add(new Framework("Sentry", Path.Combine(PlatformThirdPartyPath, "Sentry.embeddedframework.zip"), null, true));

			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "Sentry_IOS_UPL.xml"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
			PublicDefinitions.Add("COCOAPODS=0");
			PublicDefinitions.Add("SENTRY_NO_UIKIT=0");
			PublicDefinitions.Add("APPLICATION_EXTENSION_API_ONLY_NO=0");
			PublicDefinitions.Add("SDK_V9=0");
			PublicDefinitions.Add("SDK_v9=0");
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));

			PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyPath, "include"));

			RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "sentry.dylib"), Path.Combine(PlatformThirdPartyPath, "bin", "sentry.dylib"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
			PublicDefinitions.Add("COCOAPODS=0");
			PublicDefinitions.Add("SENTRY_NO_UIKIT=1");
			PublicDefinitions.Add("APPLICATION_EXTENSION_API_ONLY_NO=0");
			PublicDefinitions.Add("SDK_V9=0");
			PublicDefinitions.Add("SDK_v9=0");
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
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Microsoft"));

			bool CrashpadExists = File.Exists(Path.Combine(PlatformThirdPartyPath, "Crashpad", "bin", "crashpad_handler.exe"));

			string WindowsThirdPartyPath = Path.Combine(PlatformThirdPartyPath, CrashpadExists ? "Crashpad" : "Breakpad");

			PublicIncludePaths.Add(Path.Combine(WindowsThirdPartyPath, "include"));

			PublicAdditionalLibraries.Add(Path.Combine(WindowsThirdPartyPath, "lib", "sentry.lib"));

			if (CrashpadExists)
			{
				RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "crashpad_handler.exe"), Path.Combine(WindowsThirdPartyPath, "bin", "crashpad_handler.exe"));
				RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "crashpad_wer.dll"), Path.Combine(WindowsThirdPartyPath, "bin", "crashpad_wer.dll"));

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

			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
			PublicDefinitions.Add("SENTRY_BUILD_STATIC=1");

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

			PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyPath, "include"));

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

			AddEngineThirdPartyPrivateStaticDependencies(Target, "libcurl");
		}
		else
		{
			if (GetType() == typeof(Sentry))
			{
				PublicDefinitions.Add("SENTRY_PLATFORM_NULL");

				// Exclude sources in `Private/GenericPlatform` that use sentry-native API from the build if target platform isn't supported
				// Plugin extensions that add support for more platforms (e.g. consoles) can override this define if needed
				PublicDefinitions.Add("USE_SENTRY_NATIVE=0");

				Console.WriteLine("To use Sentry SDK on game consoles follow the instructions at https://docs.sentry.io/platforms/unreal/game-consoles/");
			}
		}
	}
}
