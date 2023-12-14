// Copyright (c) 2022 Sentry. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

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

		PublicIncludePaths.AddRange(
			new string[]
			{
				// ... add public include paths required here ...
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
				// ... add other private include paths required here ...
			}
		);

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

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);

		string PlatformThirdPartyPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Source", "ThirdParty", Target.Platform.ToString()));
		string PlatformBinariesPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Binaries", Target.Platform.ToString()));

		// Additional routine for iOS
		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));

			PublicAdditionalFrameworks.Add(new Framework("Sentry", Path.Combine(PlatformThirdPartyPath, "Sentry.embeddedframework.zip"), null, true));

			PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "Sentry_IOS_UPL.xml"));

			PublicDefinitions.Add("COCOAPODS=0");
			PublicDefinitions.Add("SENTRY_NO_UIKIT=1");
		}

		// Additional routine for Android
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Android"));

			PublicDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "Sentry_Android_UPL.xml"));
		}

		// Additional routine for Windows
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			bool CrashpadExists = File.Exists(Path.Combine(PlatformThirdPartyPath, "Crashpad", "bin", "crashpad_handler.exe"));

			string WindowsThirdPartyPath = Path.Combine(PlatformThirdPartyPath, CrashpadExists ? "Crashpad" : "Breakpad");

			PublicIncludePaths.Add(Path.Combine(WindowsThirdPartyPath, "include"));
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Desktop"));

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

			PublicSystemLibraries.Add("winhttp.lib");
			PublicSystemLibraries.Add("version.lib");

			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
			PublicDefinitions.Add("SENTRY_BUILD_STATIC=1");
		}

		// Additional routine for Linux
		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyPath, "include"));
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Desktop"));

			RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "crashpad_handler"), Path.Combine(PlatformThirdPartyPath, "bin", "crashpad_handler"));
			RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "libsentry.so"), Path.Combine(PlatformThirdPartyPath, "bin", "libsentry.so"));

			PublicRuntimeLibraryPaths.Add(PlatformBinariesPath);

			PublicAdditionalLibraries.Add(Path.Combine(PlatformBinariesPath, "libsentry.so"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
		}

		// Additional routine for Mac
		if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyPath, "include"));
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));

			RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "sentry.dylib"), Path.Combine(PlatformThirdPartyPath, "bin", "sentry.dylib"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
			PublicDefinitions.Add("COCOAPODS=0");
			PublicDefinitions.Add("SENTRY_NO_UIKIT=1");
		}
	}
}
