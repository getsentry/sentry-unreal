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

		// Initial values must match the UPROPERTY defaults in SentrySettings to handle the case
		// when the user hasn't explicitly configured them and no entry exists in the .ini file
		bool bEnableExternalCrashReporter = false;
		bool bUseNativeBackend = false;

		if (Target.ProjectFile != null)
		{
			ConfigHierarchy EngineConfig = ConfigCache.ReadHierarchy(ConfigHierarchyType.Engine, DirectoryReference.FromFile(Target.ProjectFile), Target.Platform);
			EngineConfig.GetBool("/Script/Sentry.SentrySettings", "EnableExternalCrashReporter", out bEnableExternalCrashReporter);
			EngineConfig.GetBool("/Script/Sentry.SentrySettings", "UseNativeBackend", out bUseNativeBackend);
		}

		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));

			PublicAdditionalFrameworks.Add(new Framework("Sentry", Path.Combine(PlatformThirdPartyPath, "Sentry.embeddedframework.zip"), null, true));

			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "Sentry_IOS_UPL.xml"));

			PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
			PublicDefinitions.Add("COCOAPODS=0");
			PublicDefinitions.Add("SENTRY_NO_UIKIT=0");
			PublicDefinitions.Add("SENTRY_NO_UI_FRAMEWORK=0");
			PublicDefinitions.Add("APPLICATION_EXTENSION_API_ONLY_NO=0");
			PublicDefinitions.Add("SDK_V9=0");
			PublicDefinitions.Add("SWIFT_PACKAGE=0");
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			if (bUseNativeBackend)
			{
				PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyPath, "Native", "include"));

				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, "Native", "lib", "libsentry.a"));

				RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "sentry-crash"), Path.Combine(PlatformThirdPartyPath, "Native", "bin", "sentry-crash"));

				if (bEnableExternalCrashReporter)
				{
					RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "Sentry.CrashReporter"), Path.Combine(PlatformThirdPartyPath, "Sentry.CrashReporter"));
				}

				PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
				PublicDefinitions.Add("SENTRY_BUILD_STATIC=1");

				AddEngineThirdPartyPrivateStaticDependencies(Target, "libcurl");
				AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL");
				AddEngineThirdPartyPrivateStaticDependencies(Target, "nghttp2");
			}
			else
			{
				PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Apple"));

				PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyPath, "include"));

				RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "sentry.dylib"), Path.Combine(PlatformThirdPartyPath, "bin", "sentry.dylib"));

				PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
				PublicDefinitions.Add("COCOAPODS=0");
				PublicDefinitions.Add("SENTRY_NO_UIKIT=1");
				PublicDefinitions.Add("SENTRY_NO_UI_FRAMEWORK=0");
				PublicDefinitions.Add("APPLICATION_EXTENSION_API_ONLY_NO=0");
				PublicDefinitions.Add("SDK_V9=0");
				PublicDefinitions.Add("SWIFT_PACKAGE=0");
			}
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

#if UE_5_2_OR_LATER
			if (Target.Architecture == UnrealArch.Arm64)
			{
				PlatformThirdPartyPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Source", "ThirdParty", "WinArm64"));
				PlatformBinariesPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Binaries", "WinArm64"));
			}
#endif

			string Backend = bUseNativeBackend ? "Native" : "Crashpad";

			PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyPath, Backend, "include"));

			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "sentry.lib"));

			if (bUseNativeBackend)
			{
				RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "sentry-crash.exe"), Path.Combine(PlatformThirdPartyPath, Backend, "bin", "sentry-crash.exe"));
			}
			else
			{
				RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "crashpad_handler.exe"), Path.Combine(PlatformThirdPartyPath, Backend, "bin", "crashpad_handler.exe"));
				RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "crashpad_wer.dll"), Path.Combine(PlatformThirdPartyPath, Backend, "bin", "crashpad_wer.dll"));

				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "crashpad_compat.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "crashpad_snapshot.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "crashpad_util.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "mini_chromium.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "crashpad_client.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "crashpad_zlib.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "crashpad_getopt.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "crashpad_tools.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "crashpad_handler_lib.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "crashpad_mpack.lib"));
			}

			if (bEnableExternalCrashReporter)
			{
				RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "Sentry.CrashReporter.exe"), Path.Combine(PlatformThirdPartyPath, "Sentry.CrashReporter.exe"));
			}

			PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
			PublicDefinitions.Add("SENTRY_BUILD_STATIC=1");
			PublicSystemLibraries.Add("winhttp.lib");
			PublicSystemLibraries.Add("version.lib");
			PublicSystemLibraries.Add("Synchronization.lib");
		}
#if UE_5_0_OR_LATER
		else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxArm64)
#else
		else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.LinuxAArch64)
#endif
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private", "Linux"));

			string Backend = bUseNativeBackend ? "Native" : "Crashpad";

			PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyPath, Backend, "include"));

			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "libsentry.a"));

			PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "libunwind.a"));

			if (bUseNativeBackend)
			{
				RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "sentry-crash"), Path.Combine(PlatformThirdPartyPath, Backend, "bin", "sentry-crash"));
			}
			else
			{
				RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "crashpad_handler"), Path.Combine(PlatformThirdPartyPath, Backend, "bin", "crashpad_handler"));

				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "libcrashpad_client.a"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "libcrashpad_compat.a"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "libcrashpad_handler_lib.a"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "libcrashpad_minidump.a"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "libcrashpad_snapshot.a"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "libcrashpad_tools.a"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "libcrashpad_util.a"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "libmini_chromium.a"));
				PublicAdditionalLibraries.Add(Path.Combine(PlatformThirdPartyPath, Backend, "lib", "libcrashpad_mpack.a"));
			}

			if (bEnableExternalCrashReporter)
			{
				RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "Sentry.CrashReporter"), Path.Combine(PlatformThirdPartyPath, "Sentry.CrashReporter"));
			}

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
