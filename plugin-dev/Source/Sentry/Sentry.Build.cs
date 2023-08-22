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

		string PlatformThirdPartyPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "ThirdParty", Target.Platform.ToString()));
		string PlatformBinariesPath = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "Binaries", Target.Platform.ToString()));

		// Additional routine for iOS
		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));

			PublicAdditionalFrameworks.Add(new Framework("Sentry", Path.Combine(PlatformThirdPartyPath, "Sentry.embeddedframework.zip")));

			PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "Sentry_IOS_UPL.xml"));

			PublicDefinitions.Add("COCOAPODS=0");
		}

		// Additional routine for Android
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Android"));

			PublicDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "Sentry_Android_UPL.xml"));
		}

		// Additional routine for Desktop platforms
		if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac || Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyPath, "include"));
		}

		// Additional routine for Windows
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Desktop"));

			RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "sentry.dll"), Path.Combine(PlatformThirdPartyPath, "bin", "sentry.dll"));
			RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "sentry.pdb"), Path.Combine(PlatformThirdPartyPath, "bin", "sentry.pdb"));

			PublicDelayLoadDLLs.Add("sentry.dll");

			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "lib", "sentry.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "lib", "breakpad_client.lib"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
		}

		// Additional routine for Linux
		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Desktop"));

			RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "libsentry.so"), Path.Combine(PlatformThirdPartyPath, "bin", "libsentry.so"));

			PublicRuntimeLibraryPaths.Add(PlatformBinariesPath);

			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "bin", "libsentry.so"));

			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "lib", "libbreakpad_client.a"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
		}

		// Additional routine for Mac
		if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));

			RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "sentry.dylib"), Path.Combine(PlatformThirdPartyPath, "bin", "sentry.dylib"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
			PublicDefinitions.Add("COCOAPODS=0");
		}
	}
}
