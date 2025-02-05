using UnrealBuildTool;
using System.IO;
using System;

public class SentryXbox : ModuleRules
{
	public SentryXbox(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		
		PublicIncludePaths.Add(Path.Combine(PluginDirectory, "Source", "Sentry", "Private", "Microsoft"));
			
		PublicDefinitions.Add("USE_SENTRY_NATIVE=1");
		PublicDefinitions.Add("SENTRY_BUILD_STATIC=1");
		PublicDefinitions.Add("USE_SENTRY_BREAKPAD=1");
		
		var cmakeTargetPath = Path.GetFullPath(Target.ProjectFile.FullName);
		var targetLocation = Directory.GetParent(cmakeTargetPath).FullName + "/Plugins/sentry/sentry-native";

		CMakeTargetInst cmakeTarget =
			new CMakeTargetInst("sentry-native", Target.Platform.ToString(), targetLocation, "");
		cmakeTarget.Load(Target, this);

		string intermediatePath =
			Path.Combine(Target.ProjectFile.Directory.FullName, "Intermediate", "CMakeTarget", "sentry-native");
			
		Console.WriteLine("Adding include path: "+targetLocation+"/include");
		PublicIncludePaths.Add(targetLocation + "/include");
		
		string buildPath = Path.Combine(intermediatePath, "XSX", "build");
		if(Target.Configuration == UnrealTargetConfiguration.Debug)
		{
			PublicAdditionalLibraries.Add(Path.Combine(buildPath, "Debug", "sentry.lib"));
		}
		else
		{
			PublicAdditionalLibraries.Add(Path.Combine(buildPath, "Release", "sentry.lib"));
		}
	}
}