using UnrealBuildTool;
using System;
using System.IO;

#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

public class SentryMac : ModuleRules
{
	public SentryMac(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		string PlatformThirdPartyPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Source", "ThirdParty", Target.Platform.ToString()));
		string PlatformBinariesPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Binaries", Target.Platform.ToString()));
		
		PublicIncludePaths.Add(Path.Combine(PlatformThirdPartyPath, "include"));

		RuntimeDependencies.Add(Path.Combine(PlatformBinariesPath, "sentry.dylib"), Path.Combine(PlatformThirdPartyPath, "bin", "sentry.dylib"));

		PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
		PublicDefinitions.Add("COCOAPODS=0");
		PublicDefinitions.Add("SENTRY_NO_UIKIT=1");
		PublicDefinitions.Add("APPLICATION_EXTENSION_API_ONLY_NO=0");
	}
}