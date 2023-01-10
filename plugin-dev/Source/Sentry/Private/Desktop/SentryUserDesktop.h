// Copyright (c) 2022 Sentry. All Rights Reserved.

#pragma once

#include "Convenience/SentryInclude.h"

#include "Interface/SentryUserInterface.h"

#if USE_SENTRY_NATIVE

class SentryUserDesktop : public ISentryUser
{
public:
	SentryUserDesktop();
	SentryUserDesktop(sentry_value_t user);
	virtual ~SentryUserDesktop() override;

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
	sentry_value_t UserDesktop;
};

#endif
