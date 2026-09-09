// Copyright (c) 2025 Sentry. All Rights Reserved.

using UnrealBuildTool;
#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

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
                "SharedSettingsWidgets"
            }
        );

#if UE_5_8_OR_LATER
        // AI toolsets are opt-in: AllToolsets is the umbrella plugin a project enables to get them, and it
        // pulls in ToolsetRegistry transitively. Checking it keeps this all-or-nothing and avoids depending
        // on a module the target doesn't have. Toolset sources live outside the module directory so that UHT
        // doesn't process them when they aren't being built.
        if (IsPluginEnabled(Target, "AllToolsets"))
        {
            ConditionalAddModuleDirectory(DirectoryReference.Combine(new DirectoryReference(PluginDirectory), "Source", "Toolsets"));

            PrivateDependencyModuleNames.Add("ToolsetRegistry");
            PrivateDefinitions.Add("SENTRY_WITH_TOOLSETS=1");
        }
#endif
    }

    private bool IsPluginEnabled(ReadOnlyTargetRules Target, string PluginName)
    {
        if (Target.ProjectFile == null)
        {
            return false;
        }

        ProjectDescriptor Project = ProjectDescriptor.FromFile(Target.ProjectFile);
        if (Project.Plugins == null)
        {
            return false;
        }

        foreach (PluginReferenceDescriptor PluginRef in Project.Plugins)
        {
            if (PluginRef.Name == PluginName && PluginRef.bEnabled)
            {
                return true;
            }
        }
        return false;
    }
}
