using UnrealBuildTool;
using System.IO;
using System;

public class SentryLinux : ModuleRules
{
	public SentryLinux(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		string PlatformThirdPartyPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Source", "ThirdParty", Target.Platform.ToString()));
		string PlatformBinariesPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Binaries", Target.Platform.ToString()));

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
	}
}