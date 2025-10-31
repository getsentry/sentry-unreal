// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Platform detection information for Wine/Proton environments
 */
struct FWineProtonInfo
{
	/** Whether Wine or Proton is detected */
	bool bIsRunningUnderWine = false;

	/** Whether running specifically under Proton (Steam's Wine fork) */
	bool bIsProton = false;

	/** Wine/Proton version string (e.g., "8.0-3" for Proton, "9.0" for Wine) */
	FString Version;

	/** Proton build name (e.g., "Proton 8.0", "Proton Experimental") */
	FString ProtonBuildName;

	/** Whether this is an experimental Proton build */
	bool bIsExperimental = false;
};
