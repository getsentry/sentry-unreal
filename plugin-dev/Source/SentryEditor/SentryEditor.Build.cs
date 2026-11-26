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
                "UATHelper",
                "SharedSettingsWidgets",
                "TargetPlatform"
            }
        );

        if (Target.Platform.IsInGroup(UnrealPlatformGroup.Windows))
        {
            AddEngineThirdPartyPrivateStaticDependencies(Target, "NVAftermath");
        }

#if UE_5_2_OR_LATER
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PrivateDependencyModuleNames.Add("FileUtilities");
        }
#endif
    }
}
