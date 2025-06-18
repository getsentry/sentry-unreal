// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "SentryDataTypes.h"
#include "SentryImplWrapper.h"
#include "SentryVariant.h"

#include "SentryEvent.generated.h"

class USentryId;
class ISentryEvent;

/**
 * Data being sent to Sentry.
 */
UCLASS(BlueprintType, NotBlueprintable, HideDropdown)
class SENTRY_API USentryEvent : public UObject, public TSentryImplWrapper<ISentryEvent, USentryEvent>
{
	GENERATED_BODY()

public:
	/** Initializes the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void Initialize();

	/** Gets id of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetId() const;

	/** Sets message of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetMessage(const FString& Message);

	/** Gets message of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetMessage() const;

	/** Sets the level of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetLevel(ESentryLevel Level);

	/** Gets the level of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	ESentryLevel GetLevel() const;

	/** Sets fingerprint of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetFingerprint(const TArray<FString>& Fingerprint);

	/** Gets fingerprint of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	TArray<FString> GetFingerprint() const;

	/** Sets a tag of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetTag(const FString& Key, const FString& Value);

	/** Gets a tag of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetTag(const FString& Key) const;

	/** Tries to get a tag of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool TryGetTag(const FString& Key, FString& Value) const;

	/** Removes a tag of the event with the specified key. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveTag(const FString& Key);

	/** Sets tags of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetTags(const TMap<FString, FString>& Tags);

	/** Gets tags of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FString> GetTags() const;

	/** Sets context values of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetContext(const FString& Key, const TMap<FString, FSentryVariant>& Values);

	/** Gets context of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FSentryVariant> GetContext(const FString& Key) const;

	/** Tries to get context of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool TryGetContext(const FString& Key, TMap<FString, FSentryVariant>& Value) const;

	/** Sets context values of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveContext(const FString& Key);

	/** Sets an extra of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetExtra(const FString& Key, const FSentryVariant& Value);

	/** Gets an extra of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FSentryVariant GetExtra(const FString& Key) const;

	/** Tries to get an extra of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool TryGetExtra(const FString& Key, FSentryVariant& Value) const;

	/** Removes the extra of the event with the specified key. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void RemoveExtra(const FString& Key);

	/** Sets extras of the event. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetExtras(const TMap<FString, FSentryVariant>& Extras);

	/** Gets extras of the event. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FSentryVariant> GetExtras() const;

	/** Gets flag indicating whether the event is a crash. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	bool IsCrash() const;

	/** Gets flag indicating whether the event is an Application Not Responding (ANR) error. */
	UFUNCTION(BlueprintPure, Category = "Sentry", Meta = (DisplayName = "Is App Not Responding"))
	bool IsAnr() const;
};
