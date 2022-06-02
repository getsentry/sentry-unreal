// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "SentryDataTypes.h"
#include "SentryScope.h"

#include "SentrySubsystem.generated.h"

class USentrySettings;
class USentryBreadcrumb;
class USentryEvent;
class USentryId;
class USentryUserFeedback;
class USentryUser;
class ISentrySubsystem;

/**
 * Sentry main API entry point.
 */
UCLASS()
class SENTRY_API USentrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Initializes Sentry SDK with values specified in ProjectSettings > Plugins > SentrySDK. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize();

	/** Closes the Sentry SDK. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Close();

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
	void AddBreadcrumbWithParams(const FString& Message, const FString& Category, const FString& Type, const TMap<FString, FString>& Data,
		ESentryLevel Level = ESentryLevel::Info);

	/**
	 * Clear all breadcrumbs of the current Scope.
	 *
	 * @note: Not supported for Mac/Windows.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void ClearBreadcrumbs();

	/**
	 * Captures the message.
	 *
	 * @param Message The message to send.
	 * @param Level The message level.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentryId* CaptureMessage(const FString& Message, ESentryLevel Level = ESentryLevel::Info);

	/**
	 * Captures the message with a configurable scope.
	 * This allows modifying a scope without affecting other events.
	 * Changing message level during scope configuration will override Level parameter value.
	 *
	 * @param Message The message to send.
	 * @param OnConfigureScope The callback to configure the scope.
	 * @param Level The message level.
	 *
	 * @note: Not supported for Mac/Windows.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", meta = (AutoCreateRefTerm = "OnCofigureScope"))
	USentryId* CaptureMessageWithScope(const FString& Message, const FConfigureScopeDelegate& OnConfigureScope, ESentryLevel Level = ESentryLevel::Info);

	/**
	 * Captures a manually created event and sends it to Sentry.
	 *
	 * @param Event The event to send to Sentry.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentryId* CaptureEvent(USentryEvent* Event);

	/**
	 * Captures a manually created event and sends it to Sentry.
	 *
	 * @param Event The event to send to Sentry.
	 * @param OnConfigureScope The callback to configure the scope.
	 *
	 * @note: Not supported for Mac/Windows.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	USentryId* CaptureEventWithScope(USentryEvent* Event, const FConfigureScopeDelegate& OnConfigureScope);

	/**
	 * Captures a user feedback.
	 *
	 * @param UserFeedback The user feedback to send to Sentry.
	 *
	 * @note: Not supported for Mac/Windows.
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
	 *
	 * @note: Not supported for Mac/Windows.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void CaptureUserFeedbackWithParams(USentryId* EventId, const FString& Email, const FString& Comments, const FString& Name);

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
	 * Configures the scope through the callback.
	 * Sentry SDK uses the Scope to attach contextual data to events.
	 *
	 * @param OnConfigureScope The callback to configure the scope.
	 *
	 * @note: Not supported for Mac/Windows.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry", meta = (AutoCreateRefTerm = "OnCofigureScope"))
	void ConfigureScope(const FConfigureScopeDelegate& OnConfigureScope);

	/**
	 * Sets context values which will be used for enriching events. 
	 *
	 * @param Key Context key.
	 * @param Values Context values.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetContext(const FString& Key, const TMap<FString, FString>& Values);

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

private:
	/** Adds default context data for all events captured by Sentry SDK. */
	void AddDefaultContext();

	/** Subscribe to specified game events in order to add corresponding breadcrumbs automatically. */
	void ConfigureBreadcrumbs();

private:
	TSharedPtr<ISentrySubsystem> SubsystemNativeImpl;
};
