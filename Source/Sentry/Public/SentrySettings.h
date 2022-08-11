// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SentrySettings.generated.h"

USTRUCT(BlueprintType)
struct FAutomaticBreadcrumbs
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Map loading started", ToolTip = "Flag indicating whether to automatically add breadcrumb when map loading starts."))
	bool bOnMapLoadingStarted = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Map loaded", ToolTip = "Flag indicating whether to automatically add breadcrumb after map was loaded."))
	bool bOnMapLoaded = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Game state class changed", ToolTip = "Flag indicating whether to automatically add breadcrumb when application code changes game state."))
	bool bOnGameStateClassChanged = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Game session ID changed", ToolTip = "Flag indicating whether to automatically add breadcrumb when application code changes the currently active game session."))
	bool bOnGameSessionIDChanged = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
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
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Core",
		Meta = (DisplayName = "DSN", ToolTip = "The DSN (Data Source Name) tells the SDK where to send the events to. Get your DSN in the Sentry dashboard."))
	FString DsnUrl;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Core",
		Meta = (DisplayName = "Release", ToolTip = "Release name which will be used for enriching events."))
	FString Release;

	UPROPERTY(Config, EditAnywhere, Category = "Misc",
		Meta = (DisplayName = "Initialize SDK automatically", ToolTip = "Flag indicating whether to automatically initialize the SDK when the app starts."))
	bool InitAutomatically;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Automatically add breadcrumbs"))
	FAutomaticBreadcrumbs AutomaticBreadcrumbs;

	UPROPERTY(Config, EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Upload debug symbols automatically", ToolTip = "Flag indicating whether to automatically upload debug symbols to Sentry when packaging the app."))
	bool UploadSymbolsAutomatically;

	UPROPERTY(Config, EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Project name", ToolTip = "Name of the project for which debug symbols should be uploaded.", EditCondition = "UploadSymbolsAutomatically"))
	FString ProjectName;

	UPROPERTY(Config, EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Organisation name", ToolTip = "Name of the organisation associated with the project.", EditCondition = "UploadSymbolsAutomatically"))
	FString OrganisationName;
	
	UPROPERTY(Config, EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Authentication token", ToolTip = "Authentication token for performing actions against Sentry API.", EditCondition = "UploadSymbolsAutomatically"))
	FString AuthToken;
};
