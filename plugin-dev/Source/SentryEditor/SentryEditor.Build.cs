// Copyright (c) 2025 Sentry. All Rights Reserved.

using UnrealBuildTool;

public class SentryEditor : ModuleRules
{
    public SentryEditor(ReadOnlyTargetRules Target) : base(Target)
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
                "Slate",
                "SlateCore",
                "EditorStyle",
                "Sentry",
                "PropertyEditor",
                "Projects",
                "HTTP",
                "UATHelper"
            }
        );
    }
}