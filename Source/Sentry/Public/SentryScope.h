// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "SentryScope.generated.h"

class ISentryScope;

UCLASS(BlueprintType)
class SENTRY_API USentryScope : public UObject
{
	GENERATED_BODY()

public:
	USentryScope();

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

	/** Clears the current scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Clear();

	void InitWithNativeImpl(TSharedPtr<ISentryScope> scopeImpl);
	TSharedPtr<ISentryScope> GetNativeImpl();

private:
	TSharedPtr<ISentryScope> ScopeNativeImpl;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FConfigureScopeDelegate, USentryScope*, Scope);
