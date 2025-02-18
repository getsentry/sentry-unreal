using UnrealBuildTool;
using System;
using System.IO;

#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

public class SentryIOS : ModuleRules
{
	public SentryIOS(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		string PlatformThirdPartyPath = Path.GetFullPath(Path.Combine(PluginDirectory, "Source", "ThirdParty", Target.Platform.ToString()));
		
		PublicAdditionalFrameworks.Add(new Framework("Sentry", Path.Combine(PlatformThirdPartyPath, "Sentry.embeddedframework.zip"), null, true));
			
		string PluginPath = Utils.MakePathRelativeTo(Path.Combine(PluginDirectory, "Source", "Sentry"), Target.RelativeEnginePath);

		AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "Sentry_IOS_UPL.xml"));

		PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
		PublicDefinitions.Add("COCOAPODS=0");
		PublicDefinitions.Add("SENTRY_NO_UIKIT=1");
		PublicDefinitions.Add("APPLICATION_EXTENSION_API_ONLY_NO=0");
	}
}