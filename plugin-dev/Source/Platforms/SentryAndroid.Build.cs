using UnrealBuildTool;
using System.IO;
using System;

public class SentryAndroid : ModuleRules
{
	public SentryAndroid(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
		
		PublicIncludePaths.Add(Path.Combine(PluginDirectory, "Source", "Sentry", "Private", "Android"));
			
		AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "Sentry_Android_UPL.xml"));
	}
}