// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SentrySettings.generated.h"

USTRUCT()
struct FAutomaticBreadcrumbs
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere,
		Meta = (DisplayName = "Map loading started", ToolTip = "Flag indicating whether to automatically add breadcrumb when map loading starts."))
	bool bOnMapLoadingStarted = false;

	UPROPERTY(Config, EditAnywhere,
		Meta = (DisplayName = "Map loaded", ToolTip = "Flag indicating whether to automatically add breadcrumb after map was loaded."))
	bool bOnMapLoaded = false;

	UPROPERTY(Config, EditAnywhere,
		Meta = (DisplayName = "Game state class changed", ToolTip = "Flag indicating whether to automatically add breadcrumb when application code changes game state."))
	bool bOnGameStateClassChanged = false;

	UPROPERTY(Config, EditAnywhere,
		Meta = (DisplayName = "Game session ID changed", ToolTip = "Flag indicating whether to automatically add breadcrumb when application code changes the currently active game session."))
	bool bOnGameSessionIDChanged = false;

	UPROPERTY(Config, EditAnywhere,
		Meta = (DisplayName = "User ativity string changed", ToolTip = "Flag indicating whether to automatically add breadcrumb when application code changes the user activity hint string for analytics, crash reports, etc."))
	bool bOnUserActivityStringChanged = false;
};

/**
 * Sentry settings used for plugin configuration.
 */
UCLASS(Config = Engine, defaultconfig)
class SENTRY_API USentrySettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Core",
		Meta = (DisplayName = "DSN", ToolTip = "The DSN (Data Source Name) tells the SDK where to send the events to. Get your DSN in the Sentry dashboard."))
	FString DsnUrl;

	UPROPERTY(Config, EditAnywhere, Category = "Misc",
		Meta = (DisplayName = "Initialize SDK automatically", ToolTip = "Flag indicating whether to automatically initialize the SDK when the app starts."))
	bool InitAutomatically;

	UPROPERTY(Config, EditAnywhere, Category = "Misc",
		Meta = (DisplayName = "Automatically add breadcrumbs"))
	FAutomaticBreadcrumbs AutomaticBreadcrumbs;
};
