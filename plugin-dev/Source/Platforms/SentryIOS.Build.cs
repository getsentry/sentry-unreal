using UnrealBuildTool;
using System.IO;
using System;

public class SentryIOS : ModuleRules
{
	public SentryIOS(ReadOnlyTargetRules Target) : base(Target)
	{
		string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
		
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "../", "Sentry", "Private", "Apple"));
			
		AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "Sentry_IOS_UPL.xml"));

		PublicDefinitions.Add("COCOAPODS=0");
		PublicDefinitions.Add("SENTRY_NO_UIKIT=1");
		PublicDefinitions.Add("APPLICATION_EXTENSION_API_ONLY_NO=0");
	}
}