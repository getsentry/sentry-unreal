// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "SentryUser.generated.h"

class ISentryUser;

/**
 * Information about the user who triggered an event.
 */
UCLASS(BlueprintType)
class SENTRY_API USentryUser : public UObject
{
	GENERATED_BODY()

public:
	USentryUser();

	/** Sets the email address of the user. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetEmail(const FString& Email);

	/** Gets the email address of the user. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetEmail() const;

	/** Sets the ID of the user. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetId(const FString& Id);

	/** Gets the ID of the user. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetId() const;

	/** Sets the username of the user. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetUsername(const FString& Username);

	/** Gets the username of the user. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetUsername() const;

	/** Sets the IP address of the user. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetIpAddress(const FString& IpAddress);

	/** Gets the IP address of the user. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	FString GetIpAddress() const;

	/** Sets additional arbitrary fields related to the user. */
	UFUNCTION(BlueprintCallable, Category = "Sentry")
	void SetData(const TMap<FString, FString>& Data);

	/** Gets additional arbitrary fields related to the user. */
	UFUNCTION(BlueprintPure, Category = "Sentry")
	TMap<FString, FString> GetData() const;

	void InitWithNativeImpl(TSharedPtr<ISentryUser> userImpl);
	TSharedPtr<ISentryUser> GetNativeImpl();

private:
	TSharedPtr<ISentryUser> UserNativeImpl;
};
