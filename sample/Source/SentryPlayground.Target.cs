// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class SentryPlaygroundTarget : TargetRules
{
	public SentryPlaygroundTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;

		MacPlatform.bUseDSYMFiles = true;
		IOSPlatform.bGeneratedSYM = true;

#if UE_5_1_OR_LATER
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
#endif

		ExtraModuleNames.AddRange( new string[] { "SentryPlayground" } );
	}
}
