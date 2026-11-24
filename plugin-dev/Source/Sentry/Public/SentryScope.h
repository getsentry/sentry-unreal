// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"
#include "SentryImplWrapper.h"
#include "SentryVariant.h"

#include "SentryScope.generated.h"

class ISentryScope;
class USentryBreadcrumb;
class USentryAttachment;

/**
 * Scope data to be sent with the event.
 */
UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentryScope : public UObject, public TSentryImplWrapper<ISentryScope, USentryScope>
{
	GENERATED_BODY()

public:
	/** Initializes the scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize();

	/** Adds a breadcrumb to the current Scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddBreadcrumb(USentryBreadcrumb* Breadcrumb);

	/** Clear all breadcrumbs of the current Scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void ClearBreadcrumbs();

	/** Adds an attachment to the current Scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void AddAttachment(USentryAttachment* Attachment);

	/** Clear all attachments of the current Scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void ClearAttachments();

	/** Sets a tag. Tags are searchable key/value string pairs attached to every event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetTag(const FString& Key, const FString& Value);

	/** Gets a tag. Tags are searchable key/value string pairs attached to every event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetTag(const FString& Key) const;

	/** Tries to get a tag. Tags are searchable key/value string pairs attached to every event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool TryGetTag(const FString& Key, FString& Value) const;

	/** Removes the tag for the specified key. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveTag(const FString& Key);

	/** Sets tags. Tags are searchable key/value string pairs attached to every event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetTags(const TMap<FString, FString>& Tags);

	/** Gets tags. Tags are searchable key/value string pairs attached to every event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FString> GetTags() const;

	/** Sets fingerprint in the scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetFingerprint(const TArray<FString>& Fingerprint);

	/** Gets fingerprint in the scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	TArray<FString> GetFingerprint() const;

	/** Sets the level in the scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetLevel(ESentryLevel Level);

	/** Gets the level in the scope. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	ESentryLevel GetLevel() const;

	/** Sets context which will be used for enriching events. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetContext(const FString& Key, const TMap<FString, FSentryVariant>& Values);

	/** Gets context which will be used for enriching events. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FSentryVariant> GetContext(const FString& Key) const;

	/** Tries to get context which will be used for enriching events. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool TryGetContext(const FString& Key, TMap<FString, FSentryVariant>& Value) const;

	/** Removes context which will be used for enriching events. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveContext(const FString& Key);

	/**
	 * Sets extra. These will be sent with every event.
	 *
	 * @note: On Android, variants with array/map values will be set as strings.
	 * Such values have to be interpreted as strings as well when retrieving using `GetExtra`, `TryGetExtra` or `GetExtras`
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetExtra(const FString& Key, const FSentryVariant& Value);

	/** Gets extra. These will be sent with every event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FSentryVariant GetExtra(const FString& Key) const;

	/** Tries to get an extra of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool TryGetExtra(const FString& Key, FSentryVariant& Value) const;

	/** Removes the extra for the specified key. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveExtra(const FString& Key);

	/**
	 * Sets extras. These will be sent with every event.
	 *
	 * @note: On Android, variants with array/map values will be set as strings.
	 * Such values have to be interpreted as strings as well when retrieving using `GetExtra`, `TryGetExtra` or `GetExtras`
	 */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetExtras(const TMap<FString, FSentryVariant>& Extras);

	/** Gets extras. These will be sent with every event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FSentryVariant> GetExtras() const;

	/** Clears the current scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Clear();
};

DECLARE_DELEGATE_OneParam(FConfigureScopeNativeDelegate, USentryScope*);
DECLARE_DYNAMIC_DELEGATE_OneParam(FConfigureScopeDelegate, USentryScope*, Scope);
