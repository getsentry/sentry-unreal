// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"

#include "SentryScope.generated.h"

class ISentryScope;

UCLASS(BlueprintType)
class SENTRYSDK_API USentryScope : public UObject
{
	GENERATED_BODY()

public:
	USentryScope();
	
	/** Sets a global tag. Tags are searchable key/value string pairs attached to every event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetTagValue(const FString& Key, const FString& Value);

	/** Removes the tag for the specified key. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveTag(const FString& Key);

	/** Sets global tags. Tags are searchable key/value string pairs attached to every event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetTags(const TMap<FString, FString>& Tags);

	/** Sets dist in the scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetDist(const FString& Dist);

	/** Sets environment in the scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetEnvironment(const FString& Environment);

	/** Sets fingerprint in the scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetFingerprint(const TArray<FString>& Fingerprint);

	/** Sets the level in the scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetLevel(ESentryLevel Level);

	/** Clears the current scope. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Clear();

	void InitWithNativeImpl(TSharedPtr<ISentryScope> scopeImpl);
	TSharedPtr<ISentryScope> GetNativeImpl();

private:
	TSharedPtr<ISentryScope> ScopeNativeImpl;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FConfigureScopeDelegate, USentryScope*, Scope);
