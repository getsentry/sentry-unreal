// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/GenericPlatformSentryInclude.h"

#include "Interface/SentryUserInterface.h"

#if USE_SENTRY_NATIVE

class FGenericPlatformSentryUser : public ISentryUser
{
public:
	FGenericPlatformSentryUser();
	FGenericPlatformSentryUser(sentry_value_t user);
	virtual ~FGenericPlatformSentryUser() override;

	sentry_value_t GetNativeObject();

	virtual void SetEmail(const FString& email) override;
	virtual FString GetEmail() const override;
	virtual void SetId(const FString& id) override;
	virtual FString GetId() const override;
	virtual void SetUsername(const FString& username) override;
	virtual FString GetUsername() const override;
	virtual void SetIpAddress(const FString& ipAddress) override;
	virtual FString GetIpAddress() const override;
	virtual void SetData(const TMap<FString, FString>& data) override;
	virtual TMap<FString, FString> GetData() override;

private:
	sentry_value_t User;
};

typedef FGenericPlatformSentryUser FPlatformSentryUser;

#endif
