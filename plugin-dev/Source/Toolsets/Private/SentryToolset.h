// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "ToolsetRegistry/ToolsetDefinition.h"

#include "SentryToolset.generated.h"

/// Inspects and configures the Sentry SDK in the running editor process.
UCLASS(BlueprintType, Hidden)
class USentryToolset : public UToolsetDefinition
{
	GENERATED_BODY()

public:
	/**
	 * Checks whether the Sentry SDK is initialized and capturing events.
	 * @return True if the SDK is running.
	 */
	UFUNCTION(meta = (AICallable), Category = "Sentry")
	static bool IsSentryEnabled();

	/**
	 * Checks whether a DSN is configured, without disclosing its value.
	 * @return True if a DSN is set in the plugin settings.
	 */
	UFUNCTION(meta = (AICallable), Category = "Sentry")
	static bool IsDsnConfigured();

	/**
	 * Sets the DSN in the plugin settings and persists it to the project's default engine config,
	 * leaving every other setting in that file untouched. The SDK keeps running with its previous
	 * settings until it's reinitialized.
	 * This should ONLY be called after getting explicit direction or permission from the user.
	 * @param Dsn The DSN to store. Must not be empty.
	 */
	UFUNCTION(meta = (AICallable), Category = "Sentry")
	static void SetDsn(const FString& Dsn);

	/**
	 * Restarts the SDK so that changes to the plugin settings take effect, shutting down the current
	 * session first if one is running. Avoids having to restart the editor.
	 * This should ONLY be called after getting explicit direction or permission from the user.
	 * @return True if the SDK is running once reinitialization completes.
	 */
	UFUNCTION(meta = (AICallable), Category = "Sentry")
	static bool ReinitializeSentry();

	/**
	 * Captures a message event from the editor, for confirming that events reach Sentry.
	 * @param Message The message to send. Must not be empty.
	 * @return The ID of the captured event, usable to look the event up in Sentry.
	 */
	UFUNCTION(meta = (AICallable), Category = "Sentry")
	static FString CaptureTestMessage(const FString& Message);
};
