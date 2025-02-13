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

public class SentryWin64Native : ModuleRules
{
	public SentryWin64Native(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		PublicIncludePaths.Add(Path.Combine(PluginDirectory, "Source", "Sentry", "Private", "Microsoft"));
			
		PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
		PublicDefinitions.Add("SENTRY_BUILD_STATIC=1");
		
		PrivateDependencyModuleNames.Add("SentryNative");
		
		var targetLocation = Path.Combine(PluginDirectory, "Source", "ThirdParty", "Native", "sentry-native");

		CMakeTargetInst cmakeTarget =
			new CMakeTargetInst("sentry-native", Target.Platform.ToString(), targetLocation, "");
		cmakeTarget.Load(Target, this);
		
		PublicIncludePaths.Add(targetLocation + "/include");
		
		string intermediatePath =
			Path.Combine(Target.ProjectFile.Directory.FullName, "Intermediate", "CMakeTarget", "sentry-native");

		string buildOutputPath = Path.Combine(PluginDirectory, "Binaries", "Win64");

		string buildPath = Path.Combine(intermediatePath, "Win64", "build");
		if (Target.Configuration == UnrealTargetConfiguration.Debug)
		{
			PublicAdditionalLibraries.Add(Path.Combine(buildPath, "Debug", "sentry.lib"));
		}
		else
		{
			PublicAdditionalLibraries.Add(Path.Combine(buildPath, "Release", "sentry.lib"));
		}

		if (!PublicDefinitions.Contains("USE_SENTRY_BREAKPAD=1"))
		{
			string crashpadBuildPath = Path.Combine(buildPath, "crashpad_build");
			if (Target.Configuration == UnrealTargetConfiguration.Debug)
			{
				if (!File.Exists(Path.Combine(buildOutputPath, "crashpad_handler.exe")))
				{
					Console.WriteLine("Copying crashpad_handler.exe");
					if (!System.IO.Directory.Exists(buildOutputPath))
					{
						System.IO.Directory.CreateDirectory(buildOutputPath);
					}

					File.Copy(Path.Combine(crashpadBuildPath, "handler", "Debug", "crashpad_handler.exe"),
						Path.Combine(buildOutputPath, "crashpad_handler.exe"));
				}
				else
				{
					Console.WriteLine("crashpad_handler.exe already exists");
				}

				RuntimeDependencies.Add(Path.Combine(buildOutputPath, "crashpad_handler.exe"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "handler", "Debug",
					"crashpad_handler_lib.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "client", "Debug",
					"crashpad_client.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "compat", "Debug",
					"crashpad_compat.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "minidump", "Debug",
					"crashpad_minidump.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "snapshot", "Debug",
					"crashpad_snapshot.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "getopt", "Debug",
					"crashpad_getopt.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "mini_chromium",
					"Debug", "mini_chromium.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "zlib", "Debug",
					"crashpad_zlib.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "tools", "Debug",
					"crashpad_tools.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "util", "Debug",
					"crashpad_util.lib"));
			}
			else
			{
				if (!File.Exists(Path.Combine(buildOutputPath, "crashpad_handler.exe")))
				{
					Console.WriteLine("Copying crashpad_handler.exe");
					if (!System.IO.Directory.Exists(buildOutputPath))
					{
						System.IO.Directory.CreateDirectory(buildOutputPath);
					}

					File.Copy(Path.Combine(crashpadBuildPath, "handler", "Release", "crashpad_handler.exe"),
						Path.Combine(buildOutputPath, "crashpad_handler.exe"));
				}
				else
				{
					Console.WriteLine("crashpad_handler.exe already exists");
				}

				RuntimeDependencies.Add(Path.Combine(buildOutputPath, "crashpad_handler.exe"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "handler", "Release",
					"crashpad_handler_lib.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "client", "Release",
					"crashpad_client.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "compat", "Release",
					"crashpad_compat.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "minidump", "Release",
					"crashpad_minidump.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "snapshot", "Release",
					"crashpad_snapshot.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "getopt",
					"Release",
					"crashpad_getopt.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "mini_chromium",
					"Release", "mini_chromium.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "third_party", "zlib", "Release",
					"crashpad_zlib.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "tools", "Release",
					"crashpad_tools.lib"));
				PublicAdditionalLibraries.Add(Path.Combine(crashpadBuildPath, "util", "Release",
					"crashpad_util.lib"));
			}
		}
	}
}