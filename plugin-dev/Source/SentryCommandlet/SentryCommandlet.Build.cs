// Copyright (c) 2025 Sentry. All Rights Reserved.

using UnrealBuildTool;

public class SentryCommandlet : ModuleRules
{
	public SentryCommandlet(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Sentry",
				"Projects"
			}
		);
	}
}