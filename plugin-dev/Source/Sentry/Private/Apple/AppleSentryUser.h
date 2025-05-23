// Copyright (c) 2025 Sentry. All Rights Reserved.

#pragma once

#include "Interface/SentryUserInterface.h"

@class SentryUser;

class FAppleSentryUser : public ISentryUser
{
public:
	FAppleSentryUser();
	FAppleSentryUser(SentryUser* user);
	virtual ~FAppleSentryUser() override;

	SentryUser* GetNativeObject();

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
	SentryUser* UserApple;
};

typedef FAppleSentryUser FPlatformSentryUser;
