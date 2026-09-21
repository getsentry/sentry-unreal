// Copyright (c) 2026 Sentry. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "SentryEditorLibrary.generated.h"

class USentrySettings;

UENUM(BlueprintType)
enum class ESentryDsnSource : uint8
{
	None,
	EditorDsn,
	Dsn,
	EnvironmentVariable
};

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
	 * @param PropertyName The name of the setting to persist, in either of the forms accepted by
	 * ResolveSettingName, for example "Dsn".
	 * @return True if the setting was found and written.
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	static bool SaveSettingToConfig(USentrySettings* Settings, FName PropertyName);

	/**
	 * Resolves a setting name to the property name declared in C++. Accepts that name as-is (for
	 * example "EnableTracing") as well as the name Unreal's Python API exposes the setting under
	 * ("enable_tracing"), including boolean settings whose "b" prefix Python drops
	 * ("require_user_consent" for "bRequireUserConsent").
	 *
	 * @param PropertyName The setting name in either form.
	 * @return The property name as declared in C++, or None if no setting matches.
	 */
	static FName ResolveSettingName(FName PropertyName);

	/**
	 * Gets where the DSN used by the SDK comes from, following the same precedence as
	 * USentrySettings::GetEffectiveDsn but without disclosing the value.
	 *
	 * @return The source of the effective DSN, or None if no DSN is available.
	 */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	static ESentryDsnSource GetDsnSource();
};
