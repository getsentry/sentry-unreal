using UnrealBuildTool;
using System;
using System.IO;

#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

public class SentryAndroid : ModuleRules
{
	public SentryAndroid(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		PublicDependencyModuleNames.AddRange(new string[] { "Launch" });
		string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

		AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "Sentry_Android_UPL.xml"));

		PublicDefinitions.Add("USE_SENTRY_NATIVE=0");
	}
}