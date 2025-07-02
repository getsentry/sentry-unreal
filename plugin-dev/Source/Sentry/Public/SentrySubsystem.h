// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "SentryDataTypes.h"
#include "SentryScope.h"
#include "SentryVariant.h"

#include "SentrySubsystem.generated.h"

class USentrySettings;
class USentryBreadcrumb;
class USentryEvent;
class USentryUserFeedback;
class USentryUser;
class USentryBeforeSendHandler;
class USentryBeforeBreadcrumbHandler;
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
	 * @param UserFeedback The user feedback to send to Sentry.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void CaptureUserFeedback(USentryUserFeedback* UserFeedback);

	/**
	 * Captures a user feedback.
	 *
	 * @param EventId The event Id.
	 * @param Email The user email.
	 * @param Comments The user comments.
	 * @param Name The optional username.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void CaptureUserFeedbackWithParams(const FString& EventId, const FString& Email, const FString& Comments, const FString& Name);

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
	 * Starts a new transaction.
	 *
	 * @param Name Transaction name.
	 * @param Operation Short code identifying the type of operation the span is measuring.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentryTransaction* StartTransaction(const FString& Name, const FString& Operation);

	/**
	 * Starts a new transaction with given context.
	 *
	 * @param Context Transaction context.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentryTransaction* StartTransactionWithContext(USentryTransactionContext* Context);

	/**
	 * Starts a new transaction with given context and timestamp.
	 * Currently setting the explicit transaction timings takes effect on Windows and Linux only.
	 * On other platforms starts transaction like regular `StartTransactionWithContext`.
	 *
	 * @param Context Transaction context.
	 * @param Timestamp Transaction timestamp (microseconds since the Unix epoch).
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentryTransaction* StartTransactionWithContextAndTimestamp(USentryTransactionContext* Context, int64 Timestamp);

	/**
	 * Starts a new transaction with given context and options.
	 *
	 * @param Context Transaction context.
	 * @param Options Transaction options.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentryTransaction* StartTransactionWithContextAndOptions(USentryTransactionContext* Context, const TMap<FString, FString>& Options);

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

private:
	TSharedPtr<ISentrySubsystem> SubsystemNativeImpl;

	TSharedPtr<FSentryOutputDevice> OutputDevice;
	TSharedPtr<FSentryErrorOutputDevice> OutputDeviceError;

	UPROPERTY()
	USentryBeforeSendHandler* BeforeSendHandler;
	UPROPERTY()
	USentryBeforeBreadcrumbHandler* BeforeBreadcrumbHandler;

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
