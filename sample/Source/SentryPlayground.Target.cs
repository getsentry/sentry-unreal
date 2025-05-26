// Copyright (c) 2025 Sentry. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SentryPlaygroundTarget : TargetRules
{
	public SentryPlaygroundTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;

#if UE_5_0_OR_LATER
		MacPlatform.bUseDSYMFiles = true;
		IOSPlatform.bGeneratedSYM = true;
#endif

#if UE_5_1_OR_LATER
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
#endif

		ExtraModuleNames.AddRange( new string[] { "SentryPlayground" } );
	}
}
