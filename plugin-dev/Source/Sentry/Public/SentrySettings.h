// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/EngineTypes.h"
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
		Meta = (DisplayName = "User activity string changed", ToolTip = "Flag indicating whether to automatically add breadcrumb when application code changes the user activity hint string for analytics, crash reports, etc."))
	bool bOnUserActivityStringChanged = false;
};

USTRUCT(BlueprintType)
struct FTagsPromotion
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Build configuration", ToolTip = "Flag indicating whether the build configuration should be promoted to a captured event's tag."))
	bool bPromoteBuildConfiguration = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Target type", ToolTip = "Flag indicating whether the target type should be promoted to a captured event's tag."))
	bool bPromoteTargetType = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Engine mode", ToolTip = "Flag indicating whether the engine mode should be promoted to a captured event's tag."))
	bool bPromoteEngineMode = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Is game", ToolTip = "Flag indicating whether the `IsGame` parameter should be promoted to a captured event's tag."))
	bool bPromoteIsGame = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Is standalone", ToolTip = "Flag indicating whether the `IsStandalone` parameter should be promoted to a captured event's tag."))
	bool bPromoteIsStandalone = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Is unattended", ToolTip = "Flag indicating whether the `IsUnattended` parameter should be promoted to a captured event's tag."))
	bool bPromoteIsUnattended = true;
};

USTRUCT(BlueprintType)
struct FEnableBuildConfigurations
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Debug", ToolTip = "Flag indicating whether event capturing should be enabled for the Debug build configuration."))
	bool bEnableDebug = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "DebugGame", ToolTip = "Flag indicating whether event capturing should be enabled for the DebugGame build configuration."))
	bool bEnableDebugGame = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Development", ToolTip = "Flag indicating whether event capturing should be enabled for the Development build configuration."))
	bool bEnableDevelopment = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Test", ToolTip = "Flag indicating whether event capturing should be enabled for the Test build configuration."))
	bool bEnableTest = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Shipping", ToolTip = "Flag indicating whether event capturing should be enabled for the Shipping build configuration."))
	bool bEnableShipping = true;
};

USTRUCT(BlueprintType)
struct FEnableBuildTargets
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Client", ToolTip = "Flag indicating whether event capturing should be enabled for the Client target type."))
	bool bEnableClient = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Game", ToolTip = "Flag indicating whether event capturing should be enabled for the Game target type."))
	bool bEnableGame = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Editor", ToolTip = "Flag indicating whether event capturing should be enabled for the Editor target type."))
	bool bEnableEditor = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Server", ToolTip = "Flag indicating whether event capturing should be enabled for the Server target type."))
	bool bEnableServer = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Program", ToolTip = "Flag indicating whether event capturing should be enabled for the Program target type."))
	bool bEnableProgram = true;
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
		Meta = (DisplayName = "Environment", ToolTip = "Environment which will be used for enriching events."))
	FString Environment;

	UPROPERTY(Config, EditAnywhere, Category = "Misc",
		Meta = (DisplayName = "Initialize SDK automatically", ToolTip = "Flag indicating whether to automatically initialize the SDK when the app starts."))
	bool InitAutomatically;

	UPROPERTY(Config, EditAnywhere, Category = "Misc",
		Meta = (DisplayName = "Enable verbose logging", ToolTip = "Flag indicating whether to enable verbose logging on desktop."))
	bool EnableVerboseLogging;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Enable for Build Configurations"))
	FEnableBuildConfigurations EnableBuildConfigurations;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Enable for Build Target Types"))
	FEnableBuildTargets EnableBuildTargets;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Automatically add breadcrumbs"))
	FAutomaticBreadcrumbs AutomaticBreadcrumbs;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Misc",
		Meta = (DisplayName = "Promote values to tags"))
	FTagsPromotion TagsPromotion;

	UPROPERTY(Config, EditAnywhere, Category = "Misc",
		Meta = (DisplayName = "Enable automatic crash capturing (Windows editor, UE 5.2+)", ToolTip = "Flag indicating whether to capture crashes automatically on Windows as an alternative to Crash Reporter."))
	bool EnableAutoCrashCapturing;

	UPROPERTY(Config, EditAnywhere, Category = "Misc",
		Meta = (DisplayName = "Attach game log to captured events", ToolTip = "Flag indicating whether to attach game log automatically to captured events. Not available in shipping builds."))
	bool EnableAutoLogAttachment;

	UPROPERTY(Config, EditAnywhere, Category = "Release & Health",
		Meta = (DisplayName = "Enable automatic session tracking ", ToolTip = "Flag indicating whether the SDK should automatically start a new session when it is initialized."))
	bool EnableAutoSessionTracking;

	UPROPERTY(Config, EditAnywhere, Category = "Release & Health",
		Meta = (DisplayName = "Session timeout, milliseconds (for Android/Apple only)", ToolTip = "Time interval after which session will be terminated if application is in background mode."))
	int32 SessionTimeout;

	UPROPERTY(Config, EditAnywhere, Category = "Release & Health",
		Meta = (InlineEditConditionToggle))
	bool OverrideReleaseName;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Release & Health",
		Meta = (DisplayName = "Override release name", ToolTip = "Release name which will be used for enriching events.", EditCondition = "OverrideReleaseName"))
	FString Release;

	UPROPERTY(Config, EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Upload debug symbols automatically", ToolTip = "Flag indicating whether to automatically upload debug symbols to Sentry when packaging the app."))
	bool UploadSymbolsAutomatically;

	UPROPERTY(EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Project Name", ToolTip = "Name of the project for which debug symbols should be uploaded.", EditCondition = "UploadSymbolsAutomatically"))
	FString ProjectName;

	UPROPERTY(EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Organization Name", ToolTip = "Name of the organisation associated with the project.", EditCondition = "UploadSymbolsAutomatically"))
	FString OrgName;

	UPROPERTY(EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Authentication token", ToolTip = "Authentication token for performing actions against Sentry API.", EditCondition = "UploadSymbolsAutomatically"))
	FString AuthToken;

	UPROPERTY(Config, EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Upload sources", ToolTip = "Flag indicating whether to automatically scan the debug files for references to source code files and upload them if any.", EditCondition = "UploadSymbolsAutomatically"))
	bool IncludeSources;

	UPROPERTY(Config, EditAnywhere, Category = "Crash Reporter",
		Meta = (DisplayName = "Crash Reporter Endpoint", ToolTip = "Endpoint that Unreal Engine Crah Reporter should use in order to upload crash data to Sentry."))
	FString CrashReporterUrl;

private:
	void LoadDebugSymbolsProperties();
};
