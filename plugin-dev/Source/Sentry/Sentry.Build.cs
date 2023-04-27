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
				"Json"
				// ... add private dependencies that you statically link with here ...
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);

		// Additional routine for iOS
		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Apple"));

			PublicAdditionalFrameworks.Add(new Framework("Sentry", "../ThirdParty/IOS/Sentry.embeddedframework.zip"));

			PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "Sentry_IOS_UPL.xml"));

			PublicDefinitions.Add("COCOAPODS=0");
		}

		// Additional routine for Android
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Android"));

			PublicDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "Sentry_Android_UPL.xml"));
		}

		// Additional routine for Desktop platforms
		string PlatformThirdPartyDir = Path.Combine(ModuleDirectory, "..", "ThirdParty", Target.Platform.ToString());
		if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac || Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyDir, "include"));
		}

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyDir, "lib", "sentry.lib"));
			PublicDelayLoadDLLs.Add("sentry.dll");

			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Desktop"));

			RuntimeDependencies.Add("$(PluginDir)/Binaries/Win64/crashpad_handler.exe", Path.Combine(PlatformThirdPartyDir, "bin/crashpad_handler.exe"));
			RuntimeDependencies.Add("$(PluginDir)/Binaries/Win64/sentry.dll", Path.Combine(PlatformThirdPartyDir, "bin/sentry.dll"));
			RuntimeDependencies.Add("$(PluginDir)/Binaries/Win64/sentry.pdb", Path.Combine(PlatformThirdPartyDir, "bin/sentry.pdb"), StagedFileType.DebugNonUFS);

			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
		}
		else if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyDir, "bin", "libsentry.so"));

			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Desktop"));

			RuntimeDependencies.Add("$(PluginDir)/Binaries/Linux/crashpad_handler", Path.Combine(PlatformThirdPartyDir, "bin/crashpad_handler"));
			RuntimeDependencies.Add("$(PluginDir)/Binaries/Linux/libsentry.so", Path.Combine(PlatformThirdPartyDir, "bin/libsentry.so"));
			RuntimeDependencies.Add("$(PluginDir)/Binaries/Linux/libsentry.dbg.so", Path.Combine(PlatformThirdPartyDir, "bin/libsentry.dbg.so"), StagedFileType.DebugNonUFS);

			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Apple"));

			RuntimeDependencies.Add("$(PluginDir)/Binaries/Mac/sentry.dylib", Path.Combine(PlatformThirdPartyDir, "bin/sentry.dylib"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
			PublicDefinitions.Add("COCOAPODS=0");
		}
	}
}
