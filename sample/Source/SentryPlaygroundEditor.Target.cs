// Copyright (c) 2025 Sentry. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SentryPlaygroundEditorTarget : TargetRules
{
	public SentryPlaygroundEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;

		// Experiment: enable Unity build and PCH files to speed up compilation in CI
		bUseUnityBuild = true;
		bUsePCHFiles = true;

#if UE_5_0_OR_LATER
		NativePointerMemberBehaviorOverride = System.Environment.GetEnvironmentVariable("DISALLOW_RAW_POINTERS") == "true"
			? PointerMemberBehavior.Disallow 
			: PointerMemberBehavior.AllowAndLog;
#endif

#if UE_5_1_OR_LATER
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
#endif

		ExtraModuleNames.AddRange( new string[] { "SentryPlayground" } );
	}
}
