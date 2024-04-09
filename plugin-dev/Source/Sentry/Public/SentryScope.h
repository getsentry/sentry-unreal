// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "SentryScope.generated.h"

class ISentryScope;
class USentryBreadcrumb;
class USentryAttachment;

/**
 * Scope data to be sent with the event.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryScope : public UObject
{
	GENERATED_BODY()

public:
	USentryScope();

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

	/** Sets a global tag. Tags are searchable key/value string pairs attached to every event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetTagValue(const FString& Key, const FString& Value);

	/** Gets a global tag. Tags are searchable key/value string pairs attached to every event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetTagValue(const FString& Key) const;

	/** Removes the tag for the specified key. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveTag(const FString& Key);

	/** Sets global tags. Tags are searchable key/value string pairs attached to every event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetTags(const TMap<FString, FString>& Tags);

	/** Gets a global tags. Tags are searchable key/value string pairs attached to every event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FString> GetTags() const;	

	/** Sets dist in the scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetDist(const FString& Dist);

	/** Gets dist in the scope. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetDist() const;

	/** Sets environment in the scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetEnvironment(const FString& Environment);

	/** Gets environment in the scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	FString GetEnvironment() const;

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

	/** Sets context values which will be used for enriching events. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetContext(const FString& Key, const TMap<FString, FString>& Values);

	/** Sets context values which will be used for enriching events. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveContext(const FString& Key);

	/** Sets a global extra. These will be sent with every event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetExtraValue(const FString& Key, const FString& Value);

	/** Gets a global extra. These will be sent with every event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetExtraValue(const FString& Key) const;

	/** Removes the extra for the specified key. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveExtra(const FString& Key);

	/** Sets global extras. These will be sent with every event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetExtras(const TMap<FString, FString>& Extras);

	/** Gets global extras. These will be sent with every event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FString> GetExtras() const;

	/** Clears the current scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Clear();

	void InitWithNativeImpl(TSharedPtr<ISentryScope> scopeImpl);
	TSharedPtr<ISentryScope> GetNativeImpl();

private:
	TSharedPtr<ISentryScope> ScopeNativeImpl;
};

DECLARE_DELEGATE_OneParam(FConfigureScopeNativeDelegate, USentryScope*);
DECLARE_DYNAMIC_DELEGATE_OneParam(FConfigureScopeDelegate, USentryScope*, Scope);
