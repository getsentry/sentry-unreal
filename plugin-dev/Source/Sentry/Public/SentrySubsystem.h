// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "SentryDataTypes.h"
#include "SentryMeasurementUnit.h"
#include "SentryScope.h"
#include "SentryTransactionOptions.h"
#include "SentryVariant.h"

#include "SentrySubsystem.generated.h"

class USentrySettings;
class USentryBreadcrumb;
class USentryEvent;
class USentryFeedback;
class USentryUser;
class USentryBeforeSendHandler;
class USentryBeforeBreadcrumbHandler;
class USentryBeforeLogHandler;
class USentryTransaction;
class USentryTraceSampler;
class USentryTransactionContext;

class ISentrySubsystem;
class FSentryOutputDevice;
class FSentryErrorOutputDevice;

DECLARE_DELEGATE_OneParam(FConfigureSettingsNativeDelegate, USentrySettings*);
DECLARE_DYNAMIC_DELEGATE_OneParam(FConfigureSettingsDelegate, USentrySettings*, Settings);

/**
 * Sentry main API entry point.
 */
UCLASS()
class SENTRY_API USentrySubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Initializes Sentry SDK with values specified in ProjectSettings > Plugins > SentrySDK. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize();

	/** Initializes Sentry SDK with values that override certain parameters specified in ProjectSettings > Plugins > SentrySDK.
	 *
	 * @param OnConfigureSettings The callback to configure the settings.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void InitializeWithSettings(const FConfigureSettingsDelegate& OnConfigureSettings);
	void InitializeWithSettings(const FConfigureSettingsNativeDelegate& OnConfigureSettings);

	/** Closes the Sentry SDK. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Close();

	/** Checks whether the Sentry SDK was initialized and event capturing is enabled. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool IsEnabled() const;

	/** Checks whether the app crashed during the last run. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	ESentryCrashedLastRun IsCrashedLastRun() const;

	/**
	 * Adds a breadcrumb to the current Scope.
	 *
	 * @param Breadcrumb The breadcrumb to send to Sentry.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddBreadcrumb(USentryBreadcrumb* Breadcrumb);

	/**
	 * Adds a breadcrumb to the current Scope.
	 *
	 * @param Message If a message is provided it’s rendered as text and the whitespace is preserved.
	 * Very long text might be abbreviated in the UI.
	 *
	 * @param Category Categories are dotted strings that indicate what the crumb is or where it comes from.
	 * Typically it’s a module name or a descriptive string. For instance ui.click could be used to indicate that a click
	 * happened in the UI or flask could be used to indicate that the event originated in the Flask framework.
	 *
	 * @param Type The type of breadcrumb.
	 * The default type is default which indicates no specific handling.
	 * Other types are currently http for HTTP requests and navigation for navigation events.
	 *
	 * @param Data Data associated with this breadcrumb.
	 * Contains a sub-object whose contents depend on the breadcrumb type.
	 * Additional parameters that are unsupported by the type are rendered as a key/value table.
	 *
	 * @param Level Breadcrumb level.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", meta = (AutoCreateRefTerm = "Data"))
	void AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FSentryVariant>& Data,
		ESentryLevel Level = ESentryLevel::Info);

	/**
	 * Add a debug level structured log message to Sentry.
	 *
	 * @param Message Log message to add.
	 * @param Category Optional category to prepend to the message.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void LogDebug(const FString& Message, const FString& Category = TEXT("LogSentrySdk"));

	/**
	 * Add a debug level structured log message to Sentry with attributes.
	 *
	 * @param Message Log message to add.
	 * @param Attributes Structured attributes to attach to the log entry.
	 * @param Category Optional category to prepend to the message.
	 *
	 * @note Attributes that have Array or Map variant types will be captured as Json string
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", meta = (AutoCreateRefTerm = "Attributes"))
	void LogDebugWithAttributes(const FString& Message, const TMap<FString, FSentryVariant>& Attributes, const FString& Category = TEXT("LogSentrySdk"));

	/**
	 * Add an info level structured log message to Sentry.
	 *
	 * @param Message Log message to add.
	 * @param Category Optional category to prepend to the message.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void LogInfo(const FString& Message, const FString& Category = TEXT("LogSentrySdk"));

	/**
	 * Add an info level structured log message to Sentry with attributes.
	 *
	 * @param Message Log message to add.
	 * @param Attributes Structured attributes to attach to the log entry.
	 * @param Category Optional category to prepend to the message.
	 *
	 * @note Attributes that have Array or Map variant types will be captured as Json string
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", meta = (AutoCreateRefTerm = "Attributes"))
	void LogInfoWithAttributes(const FString& Message, const TMap<FString, FSentryVariant>& Attributes, const FString& Category = TEXT("LogSentrySdk"));

	/**
	 * Add a warning level structured log message to Sentry.
	 *
	 * @param Message Log message to add.
	 * @param Category Optional category to prepend to the message.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void LogWarning(const FString& Message, const FString& Category = TEXT("LogSentrySdk"));

	/**
	 * Add a warning level structured log message to Sentry with attributes.
	 *
	 * @param Message Log message to add.
	 * @param Attributes Structured attributes to attach to the log entry.
	 * @param Category Optional category to prepend to the message.
	 *
	 * @note Attributes that have Array or Map variant types will be captured as Json string
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", meta = (AutoCreateRefTerm = "Attributes"))
	void LogWarningWithAttributes(const FString& Message, const TMap<FString, FSentryVariant>& Attributes, const FString& Category = TEXT("LogSentrySdk"));

	/**
	 * Add an error level structured log message to Sentry.
	 *
	 * @param Message Log message to add.
	 * @param Category Optional category to prepend to the message.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void LogError(const FString& Message, const FString& Category = TEXT("LogSentrySdk"));

	/**
	 * Add an error level structured log message to Sentry with attributes.
	 *
	 * @param Message Log message to add.
	 * @param Attributes Structured attributes to attach to the log entry.
	 * @param Category Optional category to prepend to the message.
	 *
	 * @note Attributes that have Array or Map variant types will be captured as Json string
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", meta = (AutoCreateRefTerm = "Attributes"))
	void LogErrorWithAttributes(const FString& Message, const TMap<FString, FSentryVariant>& Attributes, const FString& Category = TEXT("LogSentrySdk"));

	/**
	 * Add a fatal level structured log message to Sentry.
	 *
	 * @param Message Log message to add.
	 * @param Category Optional category to prepend to the message.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void LogFatal(const FString& Message, const FString& Category = TEXT("LogSentrySdk"));

	/**
	 * Add a fatal level structured log message to Sentry with attributes.
	 *
	 * @param Message Log message to add.
	 * @param Attributes Structured attributes to attach to the log entry.
	 * @param Category Optional category to prepend to the message.
	 *
	 * @note Attributes that have Array or Map variant types will be captured as Json string
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", meta = (AutoCreateRefTerm = "Attributes"))
	void LogFatalWithAttributes(const FString& Message, const TMap<FString, FSentryVariant>& Attributes, const FString& Category = TEXT("LogSentrySdk"));

	/**
	 * Emits a Counter metric.
	 * Counters track a value that can only be incremented.
	 *
	 * @param Key The name of the metric.
	 * @param Value The value to increment by (default 1).
	 * @param Unit The unit of measurement for the metric value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddCount(const FString& Key, int32 Value = 1, FSentryMeasurementUnit Unit = FSentryMeasurementUnit());

	/**
	 * Emits a Counter metric with attributes.
	 *
	 * @param Key The name of the metric.
	 * @param Value The value to increment by.
	 * @param Unit The unit of measurement for the metric value.
	 * @param Attributes Structured attributes to attach to the metric.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddCountWithAttributes(const FString& Key, int32 Value, FSentryMeasurementUnit Unit, const TMap<FString, FSentryVariant>& Attributes);

	/**
	 * Emits a Distribution metric.
	 * Distributions track the statistical distribution of values.
	 *
	 * @param Key The name of the metric.
	 * @param Value The value to record.
	 * @param Unit The unit of measurement for the metric value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddDistribution(const FString& Key, float Value, FSentryMeasurementUnit Unit = FSentryMeasurementUnit());

	/**
	 * Emits a Distribution metric with attributes.
	 *
	 * @param Key The name of the metric.
	 * @param Value The value to record.
	 * @param Unit The unit of measurement for the metric value.
	 * @param Attributes Structured attributes to attach to the metric.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddDistributionWithAttributes(const FString& Key, float Value, FSentryMeasurementUnit Unit, const TMap<FString, FSentryVariant>& Attributes);

	/**
	 * Emits a Gauge metric.
	 * Gauges track a value that can go up and down.
	 *
	 * @param Key The name of the metric.
	 * @param Value The current gauge value.
	 * @param Unit The unit of measurement for the metric value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddGauge(const FString& Key, float Value, FSentryMeasurementUnit Unit = FSentryMeasurementUnit());

	/**
	 * Emits a Gauge metric with attributes.
	 *
	 * @param Key The name of the metric.
	 * @param Value The current gauge value.
	 * @param Unit The unit of measurement for the metric value.
	 * @param Attributes Structured attributes to attach to the metric.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddGaugeWithAttributes(const FString& Key, float Value, FSentryMeasurementUnit Unit, const TMap<FString, FSentryVariant>& Attributes);

	/**
	 * Clear all breadcrumbs of the current Scope.
	 *
	 * @note: Not supported for Windows/Linux.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void ClearBreadcrumbs();

	/**
	 * Adds an attachment to the current Scope.
	 *
	 * @param Attachment The attachment that will be added to every event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddAttachment(USentryAttachment* Attachment);

	/** Clears all previously added attachments from the current scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void ClearAttachments();

	/**
	 * Captures the message.
	 *
	 * @param Message The message to send.
	 * @param Level The message level.
	 *
	 * @return Event ID (non-empty if successful)
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	FString CaptureMessage(const FString& Message, ESentryLevel Level = ESentryLevel::Info);

	/**
	 * Captures the message with a configurable scope.
	 * This allows modifying a scope without affecting other events.
	 * Changing message level during scope configuration will override Level parameter value.
	 *
	 * @param Message The message to send.
	 * @param OnConfigureScope The callback to configure the scope.
	 * @param Level The message level.
	 *
	 * @return Event ID (non-empty if successful)
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", meta = (AutoCreateRefTerm = "OnConfigureScope"))
	FString CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level = ESentryLevel::Info);
	FString CaptureMessageWithScope(const FString& Message, const FConfigureScopeNativeDelegate& OnConfigureScope, ESentryLevel Level = ESentryLevel::Info);

	/**
	 * Captures a manually created event and sends it to Sentry.
	 *
	 * @param Event The event to send to Sentry.
	 *
	 * @return Event ID (non-empty if successful)
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	FString CaptureEvent(USentryEvent* Event);

	/**
	 * Captures a manually created event and sends it to Sentry.
	 *
	 * @param Event The event to send to Sentry.
	 * @param OnConfigureScope The callback to configure the scope.
	 *
	 * @return Event ID (non-empty if successful)
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	FString CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeDelegate& OnConfigureScope);
	FString CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeNativeDelegate& OnConfigureScope);

	/**
	 * Captures a user feedback.
	 *
	 * @param Feedback The feedback to send to Sentry.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void CaptureFeedback(USentryFeedback* Feedback);

	/**
	 * Captures a user feedback.
	 *
	 * @param Message User feedback message (required).
	 * @param Name User name.
	 * @param Email User email.
	 * @param EventId Associated event identifier.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void CaptureFeedbackWithParams(const FString& Message, const FString& Name, const FString& Email, const FString& EventId);

	/**
	 * Sets a user for the current scope.
	 *
	 * @param User The user.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetUser(USentryUser* User);

	/** Removes a user for the current scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveUser();

	/**
	 * Sets context values which will be used for enriching events.
	 *
	 * @param Key Context key.
	 * @param Values Context values.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetContext(const FString& Key, const TMap<FString, FSentryVariant>& Values);

	/**
	 * Sets global tag - key/value string pair which will be attached to every event.
	 *
	 * @param Key Tag key.
	 * @param Value Tag value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetTag(const FString& Key, const FString& Value);

	/**
	 * Removes global tag.
	 *
	 * @param Key Tag key.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveTag(const FString& Key);

	/**
	 * Sets a global attribute that will be attached to all captured logs.
	 *
	 * @param Key Attribute key.
	 * @param Value Attribute value (supports bool, int, float, FString).
	 *
	 * @note This method is not supported on Android and will be a no-op on that platform.
	 * @note Values that have Array or Map variant types will be captured as Json string.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetAttribute(const FString& Key, const FSentryVariant& Value);

	/**
	 * Removes a global log attribute.
	 *
	 * @param Key Attribute key to remove.
	 *
	 * @note This method is not supported on Android and will be a no-op on that platform.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveAttribute(const FString& Key);

	/**
	 * Sets the level of all events sent.
	 *
	 * @param Level Event level.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetLevel(ESentryLevel Level);

	/**
	 * Starts a new session.
	 * If there's a running session, it ends it before starting the new one.
	 * This method can be used in combination with @EndSession to manually track sessions.
	 * The SDK uses sessions to inform Sentry about release and project associated project health.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void StartSession();

	/**
	 * Ends current session.
	 * This method can be used in combination with @StartSession to manually track sessions.
	 * The SDK uses sessions to inform Sentry about release and project associated project health.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void EndSession();

	/**
	 * Gives user consent for uploading crash reports.
	 *
	 * @note: This method is supported only on Windows and Linux, on other platforms it is a no-op.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void GiveUserConsent();

	/**
	 * Revokes user consent for uploading crash reports.
	 *
	 * @note: This method is supported only on Windows and Linux, on other platforms it is a no-op.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RevokeUserConsent();

	/**
	 * Returns the current user consent value.
	 *
	 * @return Current user consent value.
	 *
	 * @note: This method is supported only on Windows and Linux, on other platforms it returns default `Unknown` value.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	EUserConsent GetUserConsent() const;

	/**
	 * Returns if user consent is required for crash upload.
	 *
	 * @return True if user consent is required; otherwise false.
	 *
	 * @note This method is currently only relevant on Windows and Linux; other platforms will default to `false`.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	bool IsUserConsentRequired() const;

	/**
	 * Starts a new transaction.
	 *
	 * @param Name Transaction name.
	 * @param Operation Short code identifying the type of operation the span is measuring.
	 * @param BindToScope Flag indicating whether the SDK should bind the new transaction to the scope. Defaults to false (transaction is not bound to scope).
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentryTransaction* StartTransaction(const FString& Name, const FString& Operation, bool BindToScope = false);

	/**
	 * Starts a new transaction with given context.
	 *
	 * @param Context Transaction context.
	 * @param BindToScope Flag indicating whether the SDK should bind the new transaction to the scope. Defaults to false (transaction is not bound to scope).
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentryTransaction* StartTransactionWithContext(USentryTransactionContext* Context, bool BindToScope = false);

	/**
	 * Starts a new transaction with given context and timestamp.
	 * Currently setting the explicit transaction timings takes effect on Windows and Linux only.
	 * On other platforms starts transaction like regular `StartTransactionWithContext`.
	 *
	 * @param Context Transaction context.
	 * @param Timestamp Transaction timestamp (microseconds since the Unix epoch).
	 * @param BindToScope Flag indicating whether the SDK should bind the new transaction to the scope. Defaults to false (transaction is not bound to scope).
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentryTransaction* StartTransactionWithContextAndTimestamp(USentryTransactionContext* Context, int64 Timestamp, bool BindToScope = false);

	/**
	 * Starts a new transaction with given context and options.
	 *
	 * @param Context Transaction context.
	 * @param Options Transaction options.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentryTransaction* StartTransactionWithContextAndOptions(USentryTransactionContext* Context, const FSentryTransactionOptions& Options);

	/**
	 * Creates a transaction context to propagate distributed tracing metadata from upstream
	 * services and continue a trace based on corresponding HTTP header values.
	 *
	 * @param SentryTrace Incoming request 'sentry-trace' header
	 * @param BaggageHeaders Incoming request 'baggage' headers
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", meta = (AutoCreateRefTerm = "BaggageHeaders"))
	USentryTransactionContext* ContinueTrace(const FString& SentryTrace, const TArray<FString>& BaggageHeaders);

	/** Checks if Sentry event capturing is supported for current settings. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	bool IsSupportedForCurrentSettings() const;

	/** Retrieves the underlying native implementation. */
	TSharedPtr<ISentrySubsystem> GetNativeObject() const;

	/** Gets the before log handler instance. */
	USentryBeforeLogHandler* GetBeforeLogHandler() const;

private:
	/** Adds default context data for all events captured by Sentry SDK. */
	void AddDefaultContext();

	/** Adds GPU context data for all events captured by Sentry SDK. */
	void AddGpuContext();

	/** Adds GPU context data for all events captured by Sentry SDK. */
	void AddDeviceContext();

	/** Promote specified values to tags for all events captured by Sentry SDK. */
	void PromoteTags();

	/** Subscribe to specified game events in order to add corresponding breadcrumbs automatically. */
	void ConfigureBreadcrumbs();

	/** Unsubscribe from game events that are used for automatic breadcrumbs. */
	void DisableAutomaticBreadcrumbs();

	/** Check whether the event capturing should be enabled for the current build configuration */
	bool IsCurrentBuildConfigurationEnabled() const;

	/** Check whether the event capturing should be enabled for the current build target */
	bool IsCurrentBuildTargetEnabled() const;

	/** Check whether the event capturing should be enabled for promoted builds only */
	bool IsPromotedBuildsOnlyEnabled() const;

	/** Add custom Sentry output device to intercept logs */
	void ConfigureOutputDevice();

	/** Add custom Sentry output device to intercept errors */
	void ConfigureErrorOutputDevice();

	/** Add a structured log message with formatting */
	void AddLog(const FString& Message, ESentryLevel Level, const TMap<FString, FSentryVariant>& Attributes, const FString& Category);

private:
	TSharedPtr<ISentrySubsystem> SubsystemNativeImpl;

	TSharedPtr<FSentryOutputDevice> OutputDevice;
	TSharedPtr<FSentryErrorOutputDevice> OutputDeviceError;

	UPROPERTY()
	USentryBeforeSendHandler* BeforeSendHandler;
	UPROPERTY()
	USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler;

	UPROPERTY()
	USentryBeforeLogHandler* BeforeLogHandler;

	UPROPERTY()
	USentryTraceSampler* TraceSampler;

	FDelegateHandle PreLoadMapDelegate;
	FDelegateHandle PostLoadMapDelegate;
	FDelegateHandle GameStateChangedDelegate;
	FDelegateHandle UserActivityChangedDelegate;
	FDelegateHandle GameSessionIDChangedDelegate;

	FDelegateHandle OnAssertDelegate;
	FDelegateHandle OnEnsureDelegate;
};
