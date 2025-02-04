using UnrealBuildTool;
using System.IO;
using System;

public class SentryAndroid : ModuleRules
{
	public SentryAndroid(ReadOnlyTargetRules Target) : base(Target)
	{
		string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
	
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "../", "Sentry", "Private", "Android"));
			
		AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "Sentry_Android_UPL.xml"));
	}
}