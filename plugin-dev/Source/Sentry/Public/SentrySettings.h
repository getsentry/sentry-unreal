// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "UObject/NoExportTypes.h"
#include "SentrySettings.generated.h"

class USentryBeforeSendHandler;
class USentryBeforeBreadcrumbHandler;
class USentryTraceSampler;

UENUM(BlueprintType)
enum class ESentryTracesSamplingType : uint8
{
	// Use uniform sample rate for all transactions
	UniformSampleRate,
	// Control the sample rate based on the transaction itself and the context in which it's captured
	TracesSampler
};

UENUM(BlueprintType)
enum class ESentryCliLogLevel : uint8
{
	Trace,
	Debug,
	Info,
	Warn,
	Error
};

UENUM(BlueprintType)
enum class ESentryDatabaseLocation : uint8
{
	// Root directory of the current project - `FPaths::ProjectDir()`
	ProjectDirectory,
	// Root directory for user-specific game files - `FPaths::ProjectUserDir()`
	ProjectUserDirectory
};

USTRUCT(BlueprintType)
struct FAutomaticBreadcrumbs
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Map loading started", ToolTip = "Flag indicating whether to automatically add breadcrumb when map loading starts."))
	bool bOnMapLoadingStarted = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Map loaded", ToolTip = "Flag indicating whether to automatically add breadcrumb after map was loaded."))
	bool bOnMapLoaded = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Game state class changed", ToolTip = "Flag indicating whether to automatically add breadcrumb when application code changes game state."))
	bool bOnGameStateClassChanged = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Game session ID changed", ToolTip = "Flag indicating whether to automatically add breadcrumb when application code changes the currently active game session."))
	bool bOnGameSessionIDChanged = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "User activity string changed", ToolTip = "Flag indicating whether to automatically add breadcrumb when application code changes the user activity hint string for analytics, crash reports, etc."))
	bool bOnUserActivityStringChanged = false;
};

USTRUCT(BlueprintType)
struct FAutomaticBreadcrumbsForLogs
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Fatal", ToolTip = "Flag indicating whether to automatically add breadcrumb when printing log message with Fatal verbosity level."))
	bool bOnFatalLog = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Error", ToolTip = "Flag indicating whether to automatically add breadcrumb when printing log message with Error verbosity level."))
	bool bOnErrorLog = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Warning", ToolTip = "Flag indicating whether to automatically add breadcrumb when printing log message with Warning verbosity level."))
	bool bOnWarningLog = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Display/Log", ToolTip = "Flag indicating whether to automatically add breadcrumb when printing log message with Display/Log verbosity level."))
	bool bOnInfoLog = false;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Verbose/VeryVerbose", ToolTip = "Flag indicating whether to automatically add breadcrumb when printing log message with Verbose/VeryVerbose verbosity level."))
	bool bOnDebugLog = false;
};

USTRUCT(BlueprintType)
struct FTagsPromotion
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Build configuration", ToolTip = "Flag indicating whether the build configuration should be promoted to a captured event's tag."))
	bool bPromoteBuildConfiguration = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Target type", ToolTip = "Flag indicating whether the target type should be promoted to a captured event's tag."))
	bool bPromoteTargetType = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Engine mode", ToolTip = "Flag indicating whether the engine mode should be promoted to a captured event's tag."))
	bool bPromoteEngineMode = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Is game", ToolTip = "Flag indicating whether the `IsGame` parameter should be promoted to a captured event's tag."))
	bool bPromoteIsGame = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Is standalone", ToolTip = "Flag indicating whether the `IsStandalone` parameter should be promoted to a captured event's tag."))
	bool bPromoteIsStandalone = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Is unattended", ToolTip = "Flag indicating whether the `IsUnattended` parameter should be promoted to a captured event's tag."))
	bool bPromoteIsUnattended = true;
};

USTRUCT(BlueprintType)
struct FEnableBuildConfigurations
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Debug", ToolTip = "Flag indicating whether event capturing should be enabled for the Debug build configuration."))
	bool bEnableDebug = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "DebugGame", ToolTip = "Flag indicating whether event capturing should be enabled for the DebugGame build configuration."))
	bool bEnableDebugGame = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Development", ToolTip = "Flag indicating whether event capturing should be enabled for the Development build configuration."))
	bool bEnableDevelopment = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Test", ToolTip = "Flag indicating whether event capturing should be enabled for the Test build configuration."))
	bool bEnableTest = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Shipping", ToolTip = "Flag indicating whether event capturing should be enabled for the Shipping build configuration."))
	bool bEnableShipping = true;
};

USTRUCT(BlueprintType)
struct FEnableBuildTargets
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Client", ToolTip = "Flag indicating whether event capturing should be enabled for the Client target type."))
	bool bEnableClient = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Game", ToolTip = "Flag indicating whether event capturing should be enabled for the Game target type."))
	bool bEnableGame = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Editor", ToolTip = "Flag indicating whether event capturing should be enabled for the Editor target type."))
	bool bEnableEditor = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Server", ToolTip = "Flag indicating whether event capturing should be enabled for the Server target type."))
	bool bEnableServer = true;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
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

	UPROPERTY(Config, EditAnywhere, Category = "General",
		Meta = (DisplayName = "Initialize SDK automatically", ToolTip = "Flag indicating whether to automatically initialize the SDK when the app starts.", ConfigRestartRequired = true))
	bool InitAutomatically;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "DSN", ToolTip = "The DSN (Data Source Name) tells the SDK where to send the events to. Get your DSN in the Sentry dashboard."))
	FString Dsn;

	UPROPERTY(Config, EditAnywhere, Category = "General",
		Meta = (DisplayName = "Enable verbose logging", ToolTip = "Flag indicating whether to enable verbose logging."))
	bool Debug;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Environment", ToolTip = "Environment which will be used for enriching events."))
	FString Environment;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General",
		Meta = (DisplayName = "Distribution", ToolTip = "Distribution which will be used for enriching events."))
	FString Dist;

	UPROPERTY(Config, EditAnywhere, Category = "General",
		Meta = (DisplayName = "Sample rate", ToolTip = "Configures the sample rate for error events in the range of 0.0 to 1.0. The default is 1.0 which means that 100% of error events are sent. If set to 0.1 only 10% of error events will be sent. Events are picked randomly.", ClampMin = 0.0f, ClampMax = 1.0f))
	float SampleRate;

	UPROPERTY(Config, EditAnywhere, Category = "General|Attachments",
		Meta = (DisplayName = "Attach game log to captured events", ToolTip = "Flag indicating whether to attach game log automatically to captured events. Not available in shipping builds."))
	bool EnableAutoLogAttachment;

	UPROPERTY(Config, EditAnywhere, Category = "General|Attachments",
		Meta = (DisplayName = "Attach stack trace to captured events", ToolTip = "Flag indicating whether to attach stack trace automatically to captured events."))
	bool AttachStacktrace;

	UPROPERTY(Config, EditAnywhere, Category = "General|Attachments",
		Meta = (DisplayName = "Attach personally identifiable information", ToolTip = "Flag indicating whether to attach personally identifiable information (PII) to captured events."))
	bool SendDefaultPii;

	UPROPERTY(Config, EditAnywhere, Category = "General|Attachments",
		Meta = (DisplayName = "Attach screenshots", ToolTip = "Flag indicating whether to attach screenshot of the application when an error occurs. Currently this feature is supported for Windows and Linux only."))
	bool AttachScreenshot;

	UPROPERTY(Config, EditAnywhere, Category = "General|Attachments",
		Meta = (DisplayName = "Attach GPU dump", ToolTip = "Flag indicating whether to attach GPU crash dump when an error occurs. Currently this feature is supported for Nvidia graphics only."))
	bool AttachGpuDump;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Attachments",
		Meta = (DisplayName = "Max attachment size in bytes", Tooltip = "Max attachment size for each attachment in bytes. Default is 20 MiB compressed but this size is planned to be increased. Please also check the maximum attachment size of Relay to make sure your attachments don't get discarded there: https://docs.sentry.io/product/relay/options/"))
	int32 MaxAttachmentSize;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Breadcrumbs",
		Meta = (DisplayName = "Max breadcrumbs", Tooltip = "Total amount of breadcrumbs that should be captured."))
	int32 MaxBreadcrumbs;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Breadcrumbs",
		Meta = (DisplayName = "Automatically add breadcrumbs for events"))
	FAutomaticBreadcrumbs AutomaticBreadcrumbs;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Breadcrumbs",
		Meta = (DisplayName = "Automatically add breadcrumbs for log messages with verbosity level"))
	FAutomaticBreadcrumbsForLogs AutomaticBreadcrumbsForLogs;

	UPROPERTY(Config, EditAnywhere, Category = "General|Release & Health",
		Meta = (DisplayName = "Enable automatic session tracking ", ToolTip = "Flag indicating whether the SDK should automatically start a new session when it is initialized."))
	bool EnableAutoSessionTracking;

	UPROPERTY(Config, EditAnywhere, Category = "General|Release & Health",
		Meta = (DisplayName = "Session timeout, milliseconds (for Android/Apple only)", ToolTip = "Time interval after which session will be terminated if application is in background mode."))
	int32 SessionTimeout;

	UPROPERTY(Config, EditAnywhere, Category = "General|Release & Health",
		Meta = (InlineEditConditionToggle))
	bool OverrideReleaseName;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Release & Health",
		Meta = (DisplayName = "Override release name", ToolTip = "Release name which will be used for enriching events.", EditCondition = "OverrideReleaseName"))
	FString Release;

	UPROPERTY(Config, EditAnywhere, Category = "General|Native",
		Meta = (InlineEditConditionToggle))
	bool UseProxy;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Native",
		Meta = (DisplayName = "HTTP proxy", ToolTip = "HTTP proxy through which requests can be tunneled to Sentry.", EditCondition = "UseProxy"))
	FString ProxyUrl;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Hooks",
		Meta = (DisplayName = "Custom `beforeSend` event handler", ToolTip = "Custom handler for processing events before sending them to Sentry."))
	TSubclassOf<USentryBeforeSendHandler> BeforeSendHandler;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Hooks",
		Meta = (DisplayName = "Custom `beforeBreadcrumb` event handler", ToolTip = "Custom handler for processing breadcrumbs before adding them to the scope."))
	TSubclassOf<USentryBeforeBreadcrumbHandler> BeforeBreadcrumbHandler;

	UPROPERTY(Config, EditAnywhere, Category = "General|Windows",
		Meta = (DisplayName = "Override Windows default crash capturing mechanism (UE 5.2+)", ToolTip = "Flag indicating whether to capture crashes automatically on Windows as an alternative to Crash Reporter."))
	bool EnableAutoCrashCapturing;

	UPROPERTY(Config, EditAnywhere, Category = "General|Native",
		Meta = (DisplayName = "Sentry database location", ToolTip = "Location where Sentry stores its internal/temporary files."))
	ESentryDatabaseLocation DatabaseLocation;

	UPROPERTY(Config, EditAnywhere, Category = "General|Native",
		Meta = (DisplayName = "Delay app shutdown until crash report uploaded (for Crashpad only)", ToolTip = "Flag indicating whether Crashpad should delay application shutdown until the upload of the crash report is completed. It is useful in Docker environment where the life cycle of all processes is bound by the root process."))
	bool CrashpadWaitForUpload;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Mobile",
		Meta = (DisplayName = "In-app includes (for Android/Apple only)", Tooltip = "A list of string prefixes of module names that belong to the app."))
	TArray<FString> InAppInclude;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Mobile",
		Meta = (DisplayName = "In-app excludes (for Android/Apple only)", Tooltip = "A list of string prefixes of module names that don't belong to the app."))
	TArray<FString> InAppExclude;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Mobile",
		Meta = (DisplayName = "Enable ANR error tracking", Tooltip = "Flag indicating whether to enable tracking of ANR (app not responding) errors."))
	bool EnableAppNotRespondingTracking;

	UPROPERTY(Config, EditAnywhere, Category = "General|Performance Monitoring",
		Meta = (DisplayName = "Enable tracing", ToolTip = "Flag indicating whether to enable tracing for performance monitoring."))
	bool EnableTracing;

	UPROPERTY(Config, EditAnywhere, Category = "General|Performance Monitoring",
		Meta = (DisplayName = "Sampling type", ToolTip = "Method of controlling the sample rate for transactions.", EditCondition = "EnableTracing"))
	ESentryTracesSamplingType SamplingType;

	UPROPERTY(Config, EditAnywhere, Category = "General|Performance Monitoring",
		Meta = (DisplayName = "Traces sample rate", ToolTip = "Setting a uniform sample rate for all transactions to a number between 0.0 and 1.0. (For example, to send 20% of transactions, set TracesSampleRate to 0.2).",
			EditCondition = "EnableTracing && SamplingType == ESentryTracesSamplingType::UniformSampleRate", EditConditionHides))
	float TracesSampleRate;

	UPROPERTY(Config, EditAnywhere, Category = "General|Performance Monitoring",
		Meta = (DisplayName = "Traces sampler (for Android/Apple only)", ToolTip = "Custom handler for determining traces sample rate based on the sampling context.",
			EditCondition = "EnableTracing && SamplingType == ESentryTracesSamplingType::TracesSampler", EditConditionHides))
	TSubclassOf<USentryTraceSampler> TracesSampler;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Misc",
		Meta = (DisplayName = "Editor DSN", ToolTip = "The Editor DSN (Data Source Name) if you want to isolate editor crashes from packaged game crashes, defaults to Dsn if not provided."))
	FString EditorDsn;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Misc",
		Meta = (DisplayName = "Promote values to tags"))
	FTagsPromotion TagsPromotion;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Misc",
		Meta = (DisplayName = "Enable for Build Configurations"))
	FEnableBuildConfigurations EnableBuildConfigurations;

	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "General|Misc",
		Meta = (DisplayName = "Enable for Build Target Types"))
	FEnableBuildTargets EnableBuildTargets;

	UPROPERTY(Config, EditAnywhere, Category = "General|Misc",
		Meta = (DisplayName = "Enable for promoted builds only", ToolTip = "Flag indicating whether to enable for promoted builds only."))
	bool EnableForPromotedBuildsOnly;

	UPROPERTY(Config, EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Upload debug symbols automatically", ToolTip = "Flag indicating whether to automatically upload debug symbols to Sentry when packaging the app."))
	bool UploadSymbolsAutomatically;

	UPROPERTY(EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Project Name", ToolTip = "Name of the project for which debug symbols should be uploaded.", EditCondition = "UploadSymbolsAutomatically"))
	FString ProjectName;

	UPROPERTY(EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Organization Name", ToolTip = "Name of the organization associated with the project.", EditCondition = "UploadSymbolsAutomatically"))
	FString OrgName;

	UPROPERTY(EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Authentication token", ToolTip = "Authentication token for performing actions against Sentry API.", EditCondition = "UploadSymbolsAutomatically"))
	FString AuthToken;

	UPROPERTY(Config, EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Upload sources", ToolTip = "Flag indicating whether to automatically scan the debug files for references to source code files and upload them if any.", EditCondition = "UploadSymbolsAutomatically"))
	bool IncludeSources;

	UPROPERTY(Config, EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Diagnostic Level", ToolTip = "Logs verbosity level during symbol uploading.", EditCondition = "UploadSymbolsAutomatically"))
	ESentryCliLogLevel DiagnosticLevel;

	UPROPERTY(Config, EditAnywhere, Category = "Debug Symbols",
		Meta = (DisplayName = "Use legacy Sentry Gradle plugin (for Android only)", ToolTip = "Flag indicating whether to use legacy Sentry Gradle plugin for debug symbol upload. No engine's Gradle version bump is required if enabled. This can be used as a fallback if the newer Gradle 7.5 causing compatibility issues with other third-party plugins.",
			EditCondition = "UploadSymbolsAutomatically"))
	bool UseLegacyGradlePlugin;

	UPROPERTY(Config, EditAnywhere, Category = "Crash Reporter",
		Meta = (DisplayName = "Crash Reporter Endpoint", ToolTip = "Endpoint that Unreal Engine Crah Reporter should use in order to upload crash data to Sentry."))
	FString CrashReporterUrl;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	/**
	 * Gets the effective DSN based on current execution context.
	 *
	 * @return Editor DSN when running in the editor and one is set; otherwise, falls back to the default DSN.
	 */
	FString GetEffectiveDsn() const;

	static FString GetFormattedReleaseName();

	bool IsDirty() const;
	void ClearDirtyFlag();

private:
	FString GetDefaultEnvironmentName();

	void LoadDebugSymbolsProperties();

	bool bIsDirty;
};
