// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SentryEditorLibrary.generated.h"

class USentrySettings;

/**
 * Editor utilities for inspecting and persisting Sentry plugin settings.
 */
UCLASS()
class SENTRYEDITOR_API USentryEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Gets the settings object the SDK reads when it initializes. This is not the class default
	 * object, so changes made to the default object have no effect on the running SDK.
	 *
	 * @return The active settings, or nullptr if the Sentry module isn't loaded.
	 */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	static USentrySettings* GetActiveSettings();

	/**
	 * Persists a single setting to the project's default engine config, leaving every other entry in
	 * that file untouched.
	 *
	 * @param Settings The settings object holding the value to persist.
	 * @param PropertyName The name of the setting to persist, for example "Dsn".
	 * @return True if the setting was found and written.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static bool SaveSettingToConfig(USentrySettings* Settings, FName PropertyName);
};
