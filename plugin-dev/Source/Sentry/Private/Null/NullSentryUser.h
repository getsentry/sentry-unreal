// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryUserInterface.h"

class FNullSentryUser final : public ISentryUser
{
public:
	virtual ~FNullSentryUser() override = default;

	virtual void SetEmail(const FString& email) override {}
	virtual FString GetEmail() const override { return TEXT(""); }
	virtual void SetId(const FString& id) override {}
	virtual FString GetId() const override { return TEXT(""); }
	virtual void SetUsername(const FString& username) override {}
	virtual FString GetUsername() const override { return TEXT(""); }
	virtual void SetIpAddress(const FString& ipAddress) override {}
	virtual FString GetIpAddress() const override { return TEXT(""); }
	virtual void SetData(const TMap<FString, FString>& data) override {}
	virtual TMap<FString, FString> GetData() override { return {}; }
};

typedef FNullSentryUser FPlatformSentryUser;
