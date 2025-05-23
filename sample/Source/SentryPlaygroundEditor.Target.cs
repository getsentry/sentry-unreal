// Copyright (c) 2025 Sentry. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SentryPlaygroundEditorTarget : TargetRules
{
	public SentryPlaygroundEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;

		// Disable Unity build and PCH files to catch missing include errors in CI
		bUseUnityBuild = false;
		bUsePCHFiles = false;

#if UE_5_1_OR_LATER
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
#endif

		ExtraModuleNames.AddRange( new string[] { "SentryPlayground" } );
	}
}
