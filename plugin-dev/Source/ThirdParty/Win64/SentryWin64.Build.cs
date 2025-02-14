using UnrealBuildTool;
using System;
using System.IO;

#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

public class SentryWin64 : ModuleRules
{
	public SentryWin64(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
		PublicDefinitions.Add("SENTRY_BUILD_STATIC=1");
		
		string PlatformThirdPartyPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Source", "ThirdParty", Target.Platform.ToString()));
		string PlatformBinariesPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Binaries", Target.Platform.ToString()));
		
		bool CrashpadExists = File.Exists(Path.Combine(PlatformThirdPartyPath, "Crashpad", "bin", "crashpad_handler.exe"));

		string WindowsThirdPartyPath = Path.Combine(PlatformThirdPartyPath, CrashpadExists ? "Crashpad" : "Breakpad");

		PublicIncludePaths.Add(Path.Combine(WindowsThirdPartyPath, "include"));

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
}